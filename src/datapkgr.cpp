// implementation of datapkgr.h
#include "datapkgr.h"
#include <vector>
#include <hdf5_hl.h>
#include "H5Cpp.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <fstream>
#include <exception>
#include <regex>
// highfive includes
#include <highfive/H5File.hpp>

namespace h5=HighFive;

// forward declare helper functions
std::vector<std::string> listPureGroupNames(h5::Group groupName);
std::vector<std::string> listDatasetNames(h5::Group groupName);
void print_group_children(h5::Group groupName);
bool is_group_a_dataset(std::string childGroupToTest);
std::string get_sensor_label_from_apdm_v5_by_sensor_number(std::string filename, std::string sensorNumber);
void apdmH5FileFormatAddImuToFile(h5::File &file, imu imuToWrite);
int imuSensorStrToInt(const std::string& str);

namespace datapkgr{

    imudata readSingleImuDataFromApdmOpalH5FileByLabel(const std::string& filestr, const std::string& label){
        if (!is_apdm_h5_version5(filestr)){
            throw std::runtime_error("this is not a valid v5 apdm .h5 file");
        }
        // for h5 file structure for APDM see: http://share.apdm.com/documentation/TechnicalGuide.pdf
        h5::File file(filestr, h5::File::ReadOnly);
        h5::Group rootGroup=file.getGroup("/");
        h5::Group sensorsGroup=rootGroup.getGroup("Sensors");
        h5::Group processedGroup=rootGroup.getGroup("Processed");
        std::vector<std::string> availSensorsStr=sensorsGroup.listObjectNames();
        // now loop through availSensors and construct imu objects and write data
        bool sensorFoundByLabel=false;
        for(auto & currentLabel : availSensorsStr){
            // check if label is correct
            if (get_sensor_label_from_apdm_v5_by_sensor_number(filestr, currentLabel)==label){ // you've found the correct label, continue
                h5::Group currentSensorGroup=sensorsGroup.getGroup(currentLabel);
                //vector<string> childGroups=listPureGroupNames(currentSensorGroup);
                //print_group_children(currentSensorGroup);
                // set std::vector<double> of sensor data
                std::vector<std::vector<double>> dataAccel=get_2d_data_from_dataset(currentSensorGroup.getDataSet("Accelerometer"));
                std::vector<std::vector<double>> dataGyro=get_2d_data_from_dataset(currentSensorGroup.getDataSet("Gyroscope"));
                std::vector<std::vector<double>> dataMag=get_2d_data_from_dataset(currentSensorGroup.getDataSet("Magnetometer"));
                std::vector<double> unixTimeUtcMicroseconds=get_1d_data_from_dataset(currentSensorGroup.getDataSet("Time"));
                // now loop through and set data
                int vecLen=dataAccel.size();
                std::vector<double> ax(vecLen), ay(vecLen), az(vecLen), gx(vecLen), gy(vecLen), gz(vecLen), mx(vecLen), my(vecLen), mz(vecLen);
                std::vector<double> t(vecLen);
                // todo: pull out quat into 2d data and set it to vectors for qs, qx, qy, qz
                for (int j=0;j<vecLen;j++) {
                    ax[j] = dataAccel[j][0]; ay[j] = dataAccel[j][1]; az[j] = dataAccel[j][2];
                    gx[j] = dataGyro[j][0]; gy[j] = dataGyro[j][1]; gz[j] = dataGyro[j][2];
                    mx[j] = dataMag[j][0]; my[j] = dataMag[j][1]; mz[j] = dataMag[j][2];
                    t[j] = (unixTimeUtcMicroseconds[j] - unixTimeUtcMicroseconds[0]) / 1e6; // convert from unix time to relative time vector in seconds
                }// for loop to store data
                // now also pull out quaternion, if exists
                std::vector<std::vector<double>> q;
                bool quatExists=false;
                if(processedGroup.getGroup(currentLabel).exist("Orientation")){
                    q=get_2d_data_from_dataset(processedGroup.getGroup(currentLabel).getDataSet("Orientation"));
                    quatExists=true;
                }
                // now loop over and set qs, qx, qy, qz
                std::vector<double> qs(vecLen), qx(vecLen), qy(vecLen), qz(vecLen);
                if(quatExists) {
                    for (int k = 0; k < vecLen; k++) {
                        qs[k] = q[k][0];
                        qx[k] = q[k][1];
                        qy[k] = q[k][2];
                        qz[k] = q[k][3];
                    }
                }
                //std::vector<Eigen::Vector4d> qAPDM=rotutils::VectorVectorDoubleToVectorEigenVector(qAPDM0);
                //std::vector<gtsam::Rot3> orientation_Rot3=rotutils::QuaternionVectorToRot3Vector(qAPDM); // q[NWU->L]
                // remember: Eigen::Quaternion stores scalar component last
                // now store data in an imudata struct
                imudata dataout;
                dataout.ax=ax; dataout.ay=ay; dataout.az=az;
                dataout.gx=gx; dataout.gy=gy; dataout.gz=gz;
                dataout.mx=mx; dataout.my=my; dataout.mz=mz;
                dataout.qs=qs; dataout.qy=qy; dataout.qx=qx; dataout.qz=qz;
                dataout.relTimeSec=t; dataout.unixTimeUtcMicrosec=unixTimeUtcMicroseconds;
                if(quatExists){dataout.qs=qs; dataout.qy=qy; dataout.qx=qx; dataout.qz=qz;}
                //dataout.orientation=orientation_Rot3;
                dataout.label=get_sensor_label_from_apdm_v5_by_sensor_number(filestr, currentLabel);
                dataout.id=imuSensorStrToInt(currentLabel);
                sensorFoundByLabel=true;
                return dataout;
            } // if is the right label
        } // finished loop over sensors
        if (!sensorFoundByLabel){
            throw std::runtime_error("ERROR: sensor not found!");
        }
        return imudata(); // should never get here.
    } // end function

    void writeImuToApdmOpalH5File(const imu& imuToWrite, const std::string& h5Filename){
        // write an imu to an APDM v5 .h5 file
        // the parent directory of h5Filename must exist first. check for this.
        // this code seems to work. next steps:
        // constuct individual numbered groups under Processed/ and Sensors/, i.e., Sensors/234/ to hold the data.
        // 2d orientation data goes under /Processed/###/ Dataset 'Orientation'
        // 2d/1d sensor data goes under /Sensors/###/ Dataset Accelerometer,Barometer,Temperature,Time,Magnetometer,Gyroscope
        boost::filesystem::path h5file(h5Filename);
        boost::filesystem::create_directory(h5file.parent_path());
        try {
            std::cout<<"writing imu to "<<h5Filename<<std::endl;
            h5::File file(h5Filename, h5::File::ReadWrite | h5::File::Create | h5::File::Truncate); // we create a new hdf5 file
            apdmH5FileFormatAddImuToFile(file,imuToWrite);
        } catch (std::exception& err) {
            // catch and print any HDF5 error
            std::cout<<"error writing h5 file!"<<std::endl;
            std::cerr << err.what() << std::endl;
        }
    }

    void writeImuToApdmOpalH5File(const std::map<std::string,imu>& imuMapToWrite, const std::string& h5Filename){
        // write an imu to an APDM v5 .h5 file
        // the parent directory of h5Filename must exist first. check for this.
        // this code seems to work. next steps:
        // constuct individual numbered groups under Processed/ and Sensors/, i.e., Sensors/234/ to hold the data.
        // 2d orientation data goes under /Processed/###/ Dataset 'Orientation'
        // 2d/1d sensor data goes under /Sensors/###/ Dataset Accelerometer,Barometer,Temperature,Time,Magnetometer,Gyroscope
        boost::filesystem::path h5file(h5Filename);
        boost::filesystem::create_directory(h5file.parent_path());
        try {
            std::cout<<"writing imu map to "<<h5Filename<<std::endl;
            h5::File file(h5Filename, h5::File::ReadWrite | h5::File::Create | h5::File::Truncate); // we create a new hdf5 file
            for (auto const& x : imuMapToWrite){
                apdmH5FileFormatAddImuToFile(file,x.second);
            }
        } catch (std::exception& err) {
            // catch and print any HDF5 error
            std::cout<<"error writing h5 file!"<<std::endl;
            std::cerr << err.what() << std::endl;
        }
    }

    std::vector<std::vector<double>> makeNestedVector(const std::vector<double>& data1, const std::vector<double>& data2, const std::vector<double>& data3){
        if(data1.size()!=data2.size() || data1.size()!=data3.size()){throw std::runtime_error("inconsistent data sizes.");}
        std::vector<std::vector<double>> returnVector(data1.size());
        for(uint i=0; i<data1.size(); i++){
            std::vector<double> rowVec={data1[i],data2[i],data3[i]};
            returnVector[i]=rowVec;
        }
        return returnVector;
    }

    int apdmh5ToCsv(const std::string& apdmH5File, const std::string& csvFileToWrite){
        // take in an APDM .h5 file, convert it to an imu map, write this imu map to a csv file.
        std::map<std::string,imu> ImuMap=imu::getImuMapFromDataFile(apdmH5File);
        writeImuMapToCsv(ImuMap,csvFileToWrite);
        return 0;
    }

    int writeImuMapToCsv(const std::map<std::string,imu>& imuMapToWrite, const std::string& csvFileToWrite){
        // write as a large .csv file where multiple IMUs are concatenated horizontally next to each other with column pattern:
        // unixTimeAbs, reltimesec, gx, gy, gz, ax, ay, az, mx, my, mz
        std::vector<std::string> colHeaders={"unix time (us)","time (sec)","gx (rad/s)","gy (rad/s)","gz (rad/s)","ax (m/s^2)","ay (m/s^2)","az (m/s^2)","mx (uT)","my (uT)","mz (uT)"};
        int colsPerImu=11;
        std::ofstream myfile;
        myfile.open(csvFileToWrite);
        myfile<<"Converted from APDM *.h5 file.\n";
        // now loop through and set imu labels/id numbers with number of columns in between
        auto it = imuMapToWrite.begin();
        while(it!=imuMapToWrite.end()){
            std::vector<std::string> row(colsPerImu);
            row[0]=it->first; // make first entry the label
            row[1]="hardwareIdHere";
            for(auto & i : row){
                myfile<<i; myfile<<",";
            }
            it++;
        }
        myfile<<"\n";
        // now write data headers
        auto it2 = imuMapToWrite.begin();
        while(it2!=imuMapToWrite.end()){
            for(auto & colHeader : colHeaders){
                myfile<<colHeader; myfile<<",";
            }
            it2++;
        }
        myfile<<"\n";
        // now write actual data, looping through imus
        uint nMeas=((imuMapToWrite.begin())->second).length();
        std::map<std::string, imu>::const_iterator it3;
        for(uint measIdx=0;measIdx<nMeas;measIdx++){
            for(it3 = imuMapToWrite.begin(); it3!=imuMapToWrite.end(); it3++){
                if(it3->second.unixTimeUtc.size()==0){
                    myfile<<"nan,";
                }else{
                    myfile<<std::to_string((unsigned long) it3->second.unixTimeUtc[measIdx]); myfile<<",";
                }
                myfile<<it3->second.relTimeSec[measIdx]; myfile<<",";
                myfile<<it3->second.gx[measIdx]; myfile<<",";
                myfile<<it3->second.gy[measIdx]; myfile<<",";
                myfile<<it3->second.gz[measIdx]; myfile<<",";
                myfile<<it3->second.ax[measIdx]; myfile<<",";
                myfile<<it3->second.ay[measIdx]; myfile<<",";
                myfile<<it3->second.az[measIdx]; myfile<<",";
                myfile<<it3->second.mx[measIdx]; myfile<<",";
                myfile<<it3->second.my[measIdx]; myfile<<",";
                myfile<<it3->second.mz[measIdx]; myfile<<",";
            } // loop over imus
            myfile<<"\n";
        } // loop over measurement rows
        myfile.close();
        return 0;
    }

    int apdmCaseIdStringToInt(const std::string &caseId){
        std::cout<<"case id: "<<caseId<<std::endl;
        throw std::runtime_error("implement this or deprecate!");
        return 0;
    }

    std::map<std::string,imu> cutImuMapByIdx(const std::map<std::string,imu>& ImuMap, uint startIdx, uint stopIdx){
        std::map<std::string,imu> ImuMapCut;
        auto it = ImuMap.begin();
        while (it != ImuMap.end()) {
            imu originalImu=(it->second);
            imu cutImu=originalImu.cutImuByIdx(startIdx,stopIdx);
            std::string word = it->first;
            ImuMapCut.insert(std::pair<std::string,imu>(it->first,cutImu));
            it++; // increment the Iterator to point to next entry
        }
        return ImuMapCut;
    }

    std::vector<std::string> getAllImuLabelsInDataFile(const std::string& filestr){
        std::vector<std::string> allLabels;
        if (is_apdm_h5_version5(filestr)){
            // you have a version 5 APDM data file. we'll use this.
            h5::File file(filestr, h5::File::ReadOnly);
            h5::Group rootGroup=file.getGroup("/");
            h5::Group sensorsGroup=rootGroup.getGroup("Sensors");
            std::vector<std::string> availSensorsStr=sensorsGroup.listObjectNames();
            // now loop through availSensors and construct imu objects and write data
            for(uint i=0;i<availSensorsStr.size();i++){
                h5::Group currentSensorGroup=sensorsGroup.getGroup(availSensorsStr[i]);
                std::vector<std::string> sensorAttributes=currentSensorGroup.getGroup("Configuration").listAttributeNames();
                std::string lbl=get_sensor_label_from_apdm_v5_by_sensor_number(filestr, availSensorsStr[i]);
                allLabels.push_back(lbl);
            } // loop over sensors
            return allLabels;
        } else{
            std::cerr<<"ERROR: unrecognized data file type"<<std::endl;
            std::vector<std::string> returnErr={"ERROR"};
            return returnErr;
        }
    }

    std::vector<std::vector<double>> get_2d_data_from_dataset(const h5::DataSet& ds){
        // simple function to return 2d dataset as vector of vector of doubles
        std::vector<std::vector<double>> datavec2;
        ds.read(datavec2);
        return datavec2;
    }

    std::vector<double> get_1d_data_from_dataset(const h5::DataSet& ds){
        // simple function to return 1d dataset as vector of doubles
        std::vector<double> datavec1;
        ds.read(datavec1);
        return datavec1;
    }

    int write_1d_data_to_dataset(const h5::DataSet& ds, std::vector<double>){
        throw std::runtime_error("unimplemented");
        return 0;
    }

    bool is_apdm_h5_version5(std::string filestr){
            // is this a version 5 apdm file?
            try{
                h5::File file(filestr, h5::File::ReadOnly);
                h5::Group rootGroup=file.getGroup("/");
                std::vector<std::string> rootAttrs=rootGroup.listAttributeNames();
                h5::Attribute at=rootGroup.getAttribute(rootAttrs[0]);
                int fileFormatVer;
                at.read(fileFormatVer);
                return fileFormatVer == 5;
            }catch(std::exception& e){}
            return false;
    } // end func

} // end namespace

/* h5 access flags for ezh5::File(fileStr, flag) constructor
H5F_ACC_EXCL: If file already exists, H5Fcreate fails. If file does not exist, it is created and opened with read-write access.
H5F_ACC_TRUNC: If file already exists, file is opened with read-write access and new data overwrites existing data, destroying all prior content, i.e., file content is truncated upon opening. If file does not exist, it is created and opened with read-write access.
H5F_ACC_RDONLY: file is opened with read-only access. If file does not exist, H5Fopen fails.
H5F_ACC_RDWR: Existing file is opened with read-write access. If file does not exist, H5Fopen fails.
 */

// -------------- helper functions ---------------
int imuSensorStrToInt(const std::string& str){
    // takes in example string, like XI-0023412 and converts it to int
    std::string numeric = std::regex_replace(str, std::regex(R"([\D])"), "");
    int id=std::stoi(numeric);
    return id;
}

void apdmH5FileFormatAddImuToFile(h5::File &file, imu imuToWrite){
    std::vector<size_t> TwoDdims={imuToWrite.length(),3}; // set the sizes of the Nx3 2D datasets to write
    std::vector<size_t> OneDdims={imuToWrite.length()}; // set the sizes of the N 1D datasets to write
    std::string idStr=std::to_string(imuToWrite.id);

    bool fileFormatVerAttrExists = H5Aexists(file.getId(),"FileFormatVersion");
    if(!fileFormatVerAttrExists) {
        h5::Attribute fileFormatVerAttr = file.createAttribute<int>("FileFormatVersion", h5::DataSpace::From(5));
        fileFormatVerAttr.write(5);
    }
    h5::Group Processed=file.getGroup("/");
    if(file.exist("Processed")){
        Processed=file.getGroup("Processed");
    }else{
        Processed=file.createGroup("Processed");
    }
    h5::Group thisProcessed=Processed.createGroup(idStr);
    // todo: add processed quaternion data
    h5::Group Sensors=file.getGroup("/");
    if(file.exist("Sensors")){
        Sensors=file.getGroup("Sensors");
    }else{
        Sensors=file.createGroup("Sensors");
    }
    h5::Group thisSensor=Sensors.createGroup(idStr);
    // gyroscope
    h5::DataSet gyroDataset = thisSensor.createDataSet<double>("Gyroscope", h5::DataSpace(TwoDdims));
    std::vector<std::vector<double>> gyrovec=datapkgr::makeNestedVector(imuToWrite.gx,imuToWrite.gy,imuToWrite.gz);
    gyroDataset.write(gyrovec);
    // accelerometer
    h5::DataSet accelDataset = thisSensor.createDataSet<double>("Accelerometer", h5::DataSpace(TwoDdims));
    std::vector<std::vector<double>> accelvec=datapkgr::makeNestedVector(imuToWrite.ax,imuToWrite.ay,imuToWrite.az);
    accelDataset.write(accelvec);
    // magnetometer
    h5::DataSet magDataset = thisSensor.createDataSet<double>("Magnetometer", h5::DataSpace(TwoDdims));
    std::vector<std::vector<double>> magvec=datapkgr::makeNestedVector(imuToWrite.mx,imuToWrite.my,imuToWrite.mz);
    magDataset.write(magvec);
    // time
    h5::DataSet timeDataset = thisSensor.createDataSet<double>("Time", h5::DataSpace(OneDdims));
    timeDataset.write(imuToWrite.relTimeSec);
    // make configuration group
    h5::Group configGroup=thisSensor.createGroup("Configuration");
    h5::Attribute labelAttr=configGroup.createAttribute<std::string>("Label 0",h5::DataSpace::From(imuToWrite.label));
    labelAttr.write(imuToWrite.label);
}


std::vector<std::string> listPureGroupNames(h5::Group groupName){
        // will return valid groups (not datasets!)
        std::vector <std::string> childGroups;
        std::vector<std::string> childObjects=groupName.listObjectNames();
        for(uint i=0;i<childObjects.size();i++){
            if (!is_group_a_dataset(childObjects[i])){ // not a dataset, so add it
                childGroups.push_back(childObjects[i]);
            }
        }
        return childGroups;
}

std::vector<std::string> listDatasetNames(h5::Group groupName){
    // will return valid groups (not datasets!)
    std::vector <std::string> childGroups;
    std::vector<std::string> childObjects=groupName.listObjectNames();
    for(uint i=0;i<childObjects.size();i++){
        if (is_group_a_dataset(childObjects[i])){ // is a dataset, so add it
            childGroups.push_back(childObjects[i]);
        }
    }
    return childGroups;
}

bool is_group_a_dataset(std::string childGroupToTest){
    // todo: find a better way to do this.
    // right now I'm just specifying all known strings of datasets...
    std::vector<std::string> knownDatasetNames;
    knownDatasetNames.push_back("Accelerometer"); knownDatasetNames.push_back("Magnetometer"); knownDatasetNames.push_back("Gyroscope");
    knownDatasetNames.push_back("Temperature"); knownDatasetNames.push_back("Barometer"); knownDatasetNames.push_back("Time");
    bool is_found=find(knownDatasetNames.begin(), knownDatasetNames.end(), childGroupToTest) != knownDatasetNames.end();
    return is_found;
}


void print_group_children(h5::Group groupName){
    std::vector<std::string> groupStr=listPureGroupNames(groupName);
    std::vector<std::string> datasetStr=listDatasetNames(groupName);
    std::vector<std::string> attrStr=groupName.listAttributeNames();
    std::cout<<"Children of group: "<<groupName.getId()<<std::endl;
    for(uint i=0;i<groupStr.size();i++){
        std::cout<<"   (pure group) "<<groupStr[i]<<std::endl;
    }
    for(uint i=0;i<datasetStr.size();i++){
        std::cout<<"   (  dataset ) "<<datasetStr[i]<<std::endl;
    }
    for(uint i=0;i<attrStr.size();i++){
        std::cout<<"   (attribute ) "<<attrStr[i]<<std::endl;
    }
}

std::string get_sensor_label_from_apdm_v5_by_sensor_number(std::string filename, std::string sensorNumber){
    H5::H5File file(filename, H5F_ACC_RDONLY);
    H5::Group sensorConfigGroup = file.openGroup("/Sensors/"+sensorNumber+"/Configuration/");
    if (sensorConfigGroup.attrExists("Label 0")){
        H5::Attribute attr_date = sensorConfigGroup.openAttribute("Label 0");
        H5::StrType stype = attr_date.getStrType();
        std::string date_str;
        attr_date.read(stype, date_str);
        return date_str;
    }else{std::cerr<<"ERROR: label not found..."<<std::endl;}
    return "ERROR: label not found!";
}


std::string get_string_attribute_from_group(h5::Group groupName, std::string attrString){
    throw std::runtime_error("implement this");
    return "";
    /*
    // commenting out unused stuff right now to avoid build warnings
    std::vector<std::string> allAttributes=groupName.listAttributeNames();
    bool is_valid_attr=find(allAttributes.begin(), allAttributes.end(), attrString) != allAttributes.end();
    if (!is_valid_attr){
        std::cout<<"ERROR: attribute not found"<<std::endl;
    }
    // find the attribute string's index in allAttributes
    int attrIdx = distance(allAttributes.begin(), find(allAttributes.begin(), allAttributes.end(), attrString));
    h5::Attribute foundAttr=groupName.getAttribute(attrString);
    h5::DataType dt=foundAttr.getDataType();
    for (std::vector<std::string>::const_iterator it = allAttributes.begin(); it < allAttributes.end(); ++it) {
        std::cout << "attribute: " << *it << std::endl;
    }
    bool dateAtribExists = H5Aexists(groupName.getId(),"Label 0");
    hid_t attributeHandler2=H5Aopen(groupName.getId(),"Label 0",H5P_DEFAULT);

    auto attributeHandler=foundAttr.getId();
    std::cout<<"at hand1 v 2: "<<attributeHandler<<" | "<<attributeHandler2<<std::endl;
    hid_t atype=H5Aget_type(attributeHandler2);
    std::cout<<"atype: "<<atype<<std::endl;
    hsize_t sz = H5Aget_storage_size(attributeHandler2);
    char* readAttribute=0;
    auto attributeType=foundAttr.getDataType().getId();
    std::cout<<"reading attribute as: "<<H5Aread(attributeHandler,attributeType,&readAttribute)<<std::endl;
    std::cout<<"reading attribute2 as: "<<H5Aread(attributeHandler2,atype,&readAttribute)<<std::endl;
    bool hasAttribute=groupName.hasAttribute(attrString);
    foundAttr.read(readAttribute);


    vector<string> rootAttrs=rootGroup.listAttributeNames();
    h5::Attribute at=rootGroup.getAttribute(rootAttrs[0]);
    int fileFormatVer;
    at.read(fileFormatVer);
     */
}

//----------- end helper functions --------------//
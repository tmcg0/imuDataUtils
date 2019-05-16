// implementation of datapkgr.h
#include "datapkgr.h"
#include <vector>
#include <hdf5_hl.h>
#include "H5Cpp.h"
// highfive includes
#include "highfive/H5Attribute.hpp"
#include "highfive/H5DataSet.hpp"
#include "highfive/H5File.hpp"
#include "highfive/H5DataSpace.hpp"
#include "highfive/H5Group.hpp"
#include <exception>

namespace h5=HighFive;
//todo: migrate away from HighFive and use only the official HDF5 C++ API

// forward declare helper functions
std::vector<std::string> listPureGroupNames(h5::Group groupName);
std::vector<std::string> listDatasetNames(h5::Group groupName);
void print_group_children(h5::Group groupName);
bool is_group_a_dataset(std::string childGroupToTest);
std::string get_sensor_label_from_apdm_v5_by_sensor_number(std::string filename, std::string sensorNumber);

namespace datapkgr
{
    imudata readSingleImuDataFromApdmOpalH5FileByLabel(std::string filestr, std::string label){
        if (!is_apdm_h5_version5(filestr)){
                std::cout<<"this is not a valid v5 apdm .h5 file"<<std::endl;
        }
        // for h5 file structure for APDM see: http://share.apdm.com/documentation/TechnicalGuide.pdf
        h5::File file(filestr, h5::File::ReadOnly);
        h5::Group rootGroup=file.getGroup("/");
        h5::Group sensorsGroup=rootGroup.getGroup("Sensors");
        h5::Group processedGroup=rootGroup.getGroup("Processed");
        std::vector<std::string> availSensorsStr=sensorsGroup.listObjectNames();
        // now loop through availSensors and construct imu objects and write data
        bool sensorFoundByLabel=false;
        for(int i=0;i<availSensorsStr.size();i++){
            // check if label is correct
            if (get_sensor_label_from_apdm_v5_by_sensor_number(filestr, availSensorsStr[i])==label){
                h5::Group currentSensorGroup=sensorsGroup.getGroup(availSensorsStr[i]);
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
                for (int j=0;j<vecLen;j++) {
                    ax[j] = dataAccel[j][0]; ay[j] = dataAccel[j][1]; az[j] = dataAccel[j][2];
                    gx[j] = dataGyro[j][0]; gy[j] = dataGyro[j][1]; gz[j] = dataGyro[j][2];
                    mx[j] = dataMag[j][0]; my[j] = dataMag[j][1]; mz[j] = dataMag[j][2];
                    t[j] = (unixTimeUtcMicroseconds[j] - unixTimeUtcMicroseconds[0]) / 1e6; // convert from unix time to relative time vector in seconds
                }// for loop to store data
                // now also pull out quaternion from APDM
                std::vector<std::vector<double>> qAPDM0=get_2d_data_from_dataset(processedGroup.getGroup(availSensorsStr[i]).getDataSet("Orientation"));
                //std::vector<Eigen::Vector4d> qAPDM=rotutils::VectorVectorDoubleToVectorEigenVector(qAPDM0);
                //std::vector<gtsam::Rot3> orientation_Rot3=rotutils::QuaternionVectorToRot3Vector(qAPDM); // q[NWU->L]
                // remember: Eigen::Quaternion stores scalar component last
                // now store data in an imudata struct
                imudata dataout;
                dataout.ax=ax; dataout.ay=ay; dataout.az=az;
                dataout.gx=gx; dataout.gy=gy; dataout.gz=gz;
                dataout.mx=mx; dataout.my=my; dataout.mz=mz;
                dataout.relTimeSec=t; dataout.unixTimeUtcMicrosec=unixTimeUtcMicroseconds;
                //dataout.orientation=orientation_Rot3;
                dataout.label=get_sensor_label_from_apdm_v5_by_sensor_number(filestr, availSensorsStr[i]);
                sensorFoundByLabel=true;
                return dataout;
            } // if is the right label
        } // finished loop over sensors
        if (!sensorFoundByLabel){
            std::cerr<<"ERROR: sensor not found!"<<std::endl;
        }
    } // end function

    void writeImuToApdmOpalH5File(const imu& imuToWrite, const std::string& h5Filename){
        // write an imu to an APDM v5 .h5 file
        // the parent directory of h5Filename must exist first. check for this.
        // this code seems to work. next steps:
        // constuct individual numbered groups under Processed/ and Sensors/, i.e., Sensors/234/ to hold the data.
        // 2d orientation data goes under /Processed/###/ Dataset 'Orientation'
        // 2d/1d sensor data goes under /Sensors/###/ Dataset Accelerometer,Barometer,Temperature,Time,Magnetometer,Gyroscope
        try {
            h5::File file(h5Filename, h5::File::ReadWrite | h5::File::Create | h5::File::Truncate); // we create a new hdf5 file
            std::vector<size_t> TwoDdims(2); // set the sizes of the Nx3 2D datasets to write
            TwoDdims[0] = 2;
            TwoDdims[1] = 6;
            h5::Group Processed=file.createGroup("Processed");
            h5::Group Sensors=file.createGroup("Sensors");

            h5::DataSet dataset = file.createDataSet<double>("Gyroscope", h5::DataSpace(TwoDdims));
            double data[2][6] = {{1.1, 2.2, 3.3, 4.4, 5.5, 6.6},
                                 {11.11, 12.12, 13.13, 14.14, 15.15, 16.16}};
            dataset.write(data);

        } catch (std::exception& err) {
            // catch and print any HDF5 error
            std::cerr << err.what() << std::endl;
        }
    }

    std::vector<std::string> getAllImuLabelsInDataFile(std::string filestr){
        std::vector<std::string> allLabels;
        if (is_apdm_h5_version5(filestr)){
            // you have a version 5 APDM data file. we'll use this.
            h5::File file(filestr, h5::File::ReadOnly);
            h5::Group rootGroup=file.getGroup("/");
            h5::Group sensorsGroup=rootGroup.getGroup("Sensors");
            std::vector<std::string> availSensorsStr=sensorsGroup.listObjectNames();
            // now loop through availSensors and construct imu objects and write data
            for(int i=0;i<availSensorsStr.size();i++){
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
                    if (fileFormatVer==5){return true;}
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

std::vector<std::string> listPureGroupNames(h5::Group groupName){
        // will return valid groups (not datasets!)
        std::vector <std::string> childGroups;
        std::vector<std::string> childObjects=groupName.listObjectNames();
        for(int i=0;i<childObjects.size();i++){
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
    for(int i=0;i<childObjects.size();i++){
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
        for(int i=0;i<groupStr.size();i++){
                std::cout<<"   (pure group) "<<groupStr[i]<<std::endl;
        }
        for(int i=0;i<datasetStr.size();i++){
                std::cout<<"   (  dataset ) "<<datasetStr[i]<<std::endl;
        }
        for(int i=0;i<attrStr.size();i++){
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
    }else{std::cout<<"ERROR: label not found..."<<std::endl;}
}


std::string get_string_attribute_from_group(h5::Group groupName, std::string attrString){

    std::vector<std::string> allAttributes=groupName.listAttributeNames();
    bool is_valid_attr=find(allAttributes.begin(), allAttributes.end(), attrString) != allAttributes.end();
    if (!is_valid_attr){
        std::cout<<"ERROR: attribute not found"<<std::endl;
    }
    // find the attribute string's index in allAttributes
    int attrIdx = distance(allAttributes.begin(), find(allAttributes.begin(), allAttributes.end(), attrString));
    h5::Attribute foundAttr=groupName.getAttribute(attrString);
    h5::DataType dt=foundAttr.getDataType();
    for (std::vector<std::string>::const_iterator it =
            allAttributes.begin();
         it < allAttributes.end(); ++it) {
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
    int asdfas=234234;
    foundAttr.read(readAttribute);

    /*
    vector<string> rootAttrs=rootGroup.listAttributeNames();
    h5::Attribute at=rootGroup.getAttribute(rootAttrs[0]);
    int fileFormatVer;
    at.read(fileFormatVer);
     */
}

//----------- end helper functions --------------//
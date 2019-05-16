// a simple class to hold IMU data and various methods
#include "imu.h"
#include "datapkgr.h"


imu::imu(std::string filePath, int readIdx){
    //todo: make this
}

imu::imu(std::string filePath, std::string labelName){
    imudata id=datapkgr::readSingleImuDataFromApdmOpalH5FileByLabel(filePath, labelName);
    this->ax=id.ax; this->ay=id.ay; this->az=id.az;
    this->gx=id.gx; this->gy=id.gy; this->gz=id.gz;
    this->mx=id.mx; this->my=id.my; this->mz=id.mz;
    this->measTime=id.measTime;
    this->label=id.label;
    if(id.orientation.size()>0){ // if orientation exists
        this->orientation=id.orientation;
    }
    this->m_deltaT=id.measTime[1]-id.measTime[0];
} // end constructor

void imu::print_sensor_maxmin(){
// simple print the max and min for each sensor
    std::cout<<"max and min of each sensor:"<<std::endl;
    std::cout<<"ax: max="<<*max_element(this->ax.begin(),this->ax.end())<<" min="<<*min_element(this->ax.begin(),this->ax.end())<<std::endl;
    std::cout<<"ay: max="<<*max_element(this->ay.begin(),this->ay.end())<<" min="<<*min_element(this->ay.begin(),this->ay.end())<<std::endl;
    std::cout<<"az: max="<<*max_element(this->az.begin(),this->az.end())<<" min="<<*min_element(this->az.begin(),this->az.end())<<std::endl;
    std::cout<<"mx: max="<<*max_element(this->mx.begin(),this->mx.end())<<" min="<<*min_element(this->mx.begin(),this->mx.end())<<std::endl;
    std::cout<<"my: max="<<*max_element(this->my.begin(),this->my.end())<<" min="<<*min_element(this->my.begin(),this->my.end())<<std::endl;
    std::cout<<"mz: max="<<*max_element(this->mz.begin(),this->mz.end())<<" min="<<*min_element(this->mz.begin(),this->mz.end())<<std::endl;
    std::cout<<"gx: max="<<*max_element(this->gx.begin(),this->gx.end())<<" min="<<*min_element(this->gx.begin(),this->gx.end())<<std::endl;
    std::cout<<"gy: max="<<*max_element(this->gy.begin(),this->gy.end())<<" min="<<*min_element(this->gy.begin(),this->gy.end())<<std::endl;
    std::cout<<"gz: max="<<*max_element(this->gz.begin(),this->gz.end())<<" min="<<*min_element(this->gz.begin(),this->gz.end())<<std::endl;
}

unsigned long imu::length() const {
    return this->ax.size();
}

imu cutImuByIdx(int startIdx, int stopIdx){
    // cut imu data down and return the chopped imu--note that this copies the imu so the original input imu is not affected

}

std::map<std::string,imu> imu::getImuMapFromDataFile(std::string filestr){
    // this static method constructs an imu map where the keys are the label std::strings of the imu
    std::vector<std::string> allLabels=datapkgr::getAllImuLabelsInDataFile(filestr);
    std::map<std::string,imu> ImuMap;
    for(int i=0;i<allLabels.size();i++){ // now create map
        imu tempImu(filestr,allLabels[i]);
        ImuMap.insert(std::pair<std::string,imu>(allLabels[i],tempImu));
    }
    return ImuMap;
}

void imu::printLabelsInFile(std::string datafilestr){
    // prints all imu labels from given file
    std::vector<std::string> allLabels=datapkgr::getAllImuLabelsInDataFile(datafilestr);
    for(int i=0;i<allLabels.size();i++){ // now create map
        std::cout<<"imu label "<<i<<": "<<allLabels[i]<<std::endl;
    }
}
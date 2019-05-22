// a simple class to hold IMU data and various methods
#include "imu.h"
#include "datapkgr.h"


imu::imu(std::string filePath, int readIdx){
    //todo: make this
}

imu::imu(std::string filePath, std::string labelName){
    imudata myImuData=datapkgr::readSingleImuDataFromApdmOpalH5FileByLabel(filePath, labelName);
    this->ax=myImuData.ax; this->ay=myImuData.ay; this->az=myImuData.az;
    this->gx=myImuData.gx; this->gy=myImuData.gy; this->gz=myImuData.gz;
    this->mx=myImuData.mx; this->my=myImuData.my; this->mz=myImuData.mz;
    this->qs=myImuData.qs; this->qx=myImuData.qx; this->qy=myImuData.qy; this->qz=myImuData.qz;
    this->relTimeSec=myImuData.relTimeSec;
    this->unixTimeUtc=myImuData.unixTimeUtcMicrosec;
    this->label=myImuData.label;
    this->id=myImuData.id;
    if(myImuData.qx.size()>0){ // if orientation exists
    }
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

double imu::getDeltaT() const {
    // loop through and find average delta T (seconds)
    std::vector<double> timeDiff(this->length()-1);
    for(int i=0; i<timeDiff.size(); i++){
        timeDiff[i]=this->relTimeSec[i+1]-this->relTimeSec[i];
    }
    double averageDeltaT = accumulate( timeDiff.begin(), timeDiff.end(), 0.0)/timeDiff.size();
    std::cout<<"dt="<<averageDeltaT<<std::endl;
    return averageDeltaT;
}

std::vector<std::vector<double>> imu::quaternion(){ // turn quaternion into vector<vector<double>>
    std::vector<std::vector<double>> q(this->length());
    for(int i=0; i<this->length();i++){
        q[i]={qs[i],qx[i],qy[i],qz[i]};
    }
    return q;
}

imu imu::cutImuByIdx(const int& startIdx, const int& stopIdx){
    // cut imu data down and return the chopped imu--note that this copies the imu so the original input imu is not affected
    imu newImu=*this; // copy
    // now go through the imu data fields and chop down by indexes in the new imu
    if(newImu.gx.size()>stopIdx){newImu.gx=slice(newImu.gx,startIdx,stopIdx);}
    if(newImu.gy.size()>stopIdx){newImu.gy=slice(newImu.gy,startIdx,stopIdx);}
    if(newImu.gz.size()>stopIdx){newImu.gz=slice(newImu.gz,startIdx,stopIdx);}
    if(newImu.ax.size()>stopIdx){newImu.ax=slice(newImu.ax,startIdx,stopIdx);}
    if(newImu.ay.size()>stopIdx){newImu.ay=slice(newImu.ay,startIdx,stopIdx);}
    if(newImu.az.size()>stopIdx){newImu.az=slice(newImu.az,startIdx,stopIdx);}
    if(newImu.mx.size()>stopIdx){newImu.mx=slice(newImu.mx,startIdx,stopIdx);}
    if(newImu.my.size()>stopIdx){newImu.my=slice(newImu.my,startIdx,stopIdx);}
    if(newImu.mz.size()>stopIdx){newImu.mz=slice(newImu.mz,startIdx,stopIdx);}
    if(newImu.unixTimeUtc.size()>stopIdx){newImu.unixTimeUtc=slice(newImu.unixTimeUtc,startIdx,stopIdx);}
    if(newImu.relTimeSec.size()>stopIdx){newImu.relTimeSec=slice(newImu.relTimeSec,startIdx,stopIdx);}
    return newImu;
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

bool isUnixTimeSec(unsigned long time){
    // standard unix time is number of seconds since 12:00 am UTC on January 1, 1970.
    // 2536323034 is timestamp of May 19, 2050. note: code will error out after this date.
    unsigned long timeLimitSec=2536323034;
    if(time>0 && time<timeLimitSec){
        return true;
    }else{
        return false;
    }
}

int getNearestIdxFromUnixTimeUtc(int unixTimeUtc){
    //

}
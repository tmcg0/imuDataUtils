// checks for the following behavior:
// 1) all IMU files have the same timestamp
// 1.5) timestamp is constantly ascending
// 2) all data recordings (gyro, mag, accels) are doubles
// 3) time and data are same length
// usage: validateImuFile path/to/file.h5

#include <iostream>
#include <boost/filesystem.hpp>
#include "imu.h"
#include "datapkgr.h"

int testAllTimestampsSame(const std::map<std::string,imu>& ImuMap);
int testAllImusConstantlyAscendingTimestamps(const std::map<std::string,imu>& ImuMap);
bool assertVectorDoubleConstantlyIncreasing(std::vector<double>& a, double tol);

int main(int argc, char** argv){
    // check that exactly two arguments are entered
    if(argc!=2){
        std::cerr<<"enter 1 argument, usage: path/to/file.h5"<<std::endl;
        return 1;
    }
    // check that first argument is a valid h5 file
    std::string inputFile=argv[1];
    std::string inputFileExt=boost::filesystem::extension(inputFile);
    std::cout<<"input file is: "<<inputFile<<" with extension "<<inputFileExt<<std::endl;
    if(!boost::filesystem::exists(inputFile) || ".h5"!=inputFileExt){
        std::cerr<<"input file "<<inputFile<<" either does not exist or is not an .h5 file"<<std::endl;
        return 2;
    }
    // construct imu map from input file
    std::map<std::string,imu> ImuMap=imu::getImuMapFromDataFile(inputFile);
    std::cout<<"constructed imu map..."<<std::endl;
    // tests
    std::cout<<"testing for constantly ascending time... ";
    int a= testAllImusConstantlyAscendingTimestamps(ImuMap); std::cout<<"success!"<<std::endl;
    return 0;
}

int testAllImusConstantlyAscendingTimestamps(const std::map<std::string,imu>& ImuMap){
    //std::map<std::string,imu>::iterator it;
    bool isTimeAscending;
    for (auto const& it : ImuMap){
        std::vector<double> t=(it.second).relTimeSec;
        isTimeAscending=assertVectorDoubleConstantlyIncreasing(t,1.0e-5);
        if(!isTimeAscending){
            std::cerr<<"error: at least one IMU does not have constantly ascending time!"<<std::endl;
            return 1;
        }
    }
    return 0;
}

int testAllTimestampsSame(const std::map<std::string,imu>& ImuMap){

    return 0;
}

bool assertVectorDoubleConstantlyIncreasing(std::vector<double>& a, double tol){
    // loop through vector and assert that each diff is the same
    double diff=a[1]-a[0];
    for(unsigned long i=1; i<a.size(); i++){
        if(((a[i+1]-a[i])-diff)>tol){
            std::cerr<<"not all time offsets the same. difference between first diff and last diff is"<<((a[i+1]-a[i])-diff)<<std::endl;
            return false;
        }
    }
    return true;
}
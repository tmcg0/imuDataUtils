#include <iostream>
#include <boost/filesystem.hpp>
#include "imu.h"
#include "datapkgr.h"

// usage: cutImuH5ByUnixTimestamps path/to/file.h5 file/to/save.h5 (int)StartTime (int)StopTime

int main(int argc, char** argv){
    // check that exactly five arguments are entered
    if(argc!=5){
        std::cerr<<"enter 4 arguments! usage: path/to/file.h5 file/to/save.h5 (int)StartIdx (int)StopIdx"<<std::endl;
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
    // check that second argument is an h5 file and its parent directory exists (create if not)
    boost::filesystem::path outputFile(argv[2]);
    boost::filesystem::create_directory(outputFile.parent_path());
    std::string outputFileExt=boost::filesystem::extension(outputFile);
    if(".h5"!=outputFileExt){
        std::cerr<<"output file "<<outputFile<<" does not represent a valid .h5 file"<<std::endl;
        return 3;
    }
    // construct integers for third and fourth arguments
    double startTime=(double)strtol(argv[3],nullptr,0);
    double stopTime=(double)strtol(argv[4],nullptr,0);
    // construct imu map from input file
    std::map<std::string,imu> ImuMap=imu::getImuMapFromDataFile(inputFile);
    // check that both unix times are within (inclusive) the times of the IMU
    if(((ImuMap.begin())->second).unixTimeUtc.size()==0){ // unix time has not been read
        std::cerr<<"unix time is not yet read for this IMU!"<<std::endl;
        return 4;
    }
    double minStartTime=((ImuMap.begin())->second).unixTimeUtc[0];
    double maxStopTime=((ImuMap.begin())->second).unixTimeUtc[((ImuMap.begin())->second).length()-1];
    if(startTime<minStartTime){
        std::cerr<<"start time must be greater than or equal to "<<std::to_string((unsigned long) minStartTime)<<" (input start time = "<<startTime<<")"<<std::endl;
        return 5;
    }
    if(stopTime>maxStopTime){
        std::cerr<<"stop time must be less than or equal to "<<std::to_string((unsigned long) maxStopTime)<<" (input stop time = "<<stopTime<<")"<<std::endl;
        return 6;
    }
    // todo: get closest associated indexes and cut by index
    uint startTimeIdx=datapkgr::getNearestIdxFromVector<double>(ImuMap.begin()->second.unixTimeUtc, startTime);
    uint stopTimeIdx=datapkgr::getNearestIdxFromVector<double>(ImuMap.begin()->second.unixTimeUtc, stopTime);
    // now cut imu map
    std::map<std::string,imu> ImuMapCut=datapkgr::cutImuMapByIdx(ImuMap,startTimeIdx,stopTimeIdx);
    // write imu map to output h5 file
    datapkgr::writeImuToApdmOpalH5File(ImuMapCut,outputFile.string());
    return 0;
}
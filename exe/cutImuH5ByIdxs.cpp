#include <iostream>
#include <boost/filesystem.hpp>
#include "imu.h"
#include "datapkgr.h"

// usage: cutImuH5ByIdxs path/to/file.h5 file/to/save.h5 (int)StartIdx (int)StopIdx

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
    int startIdx=atoi(argv[3]);
    int stopIdx=atoi(argv[4]);
    // construct imu map from input file
    std::map<std::string,imu> ImuMap=imu::getImuMapFromDataFile(inputFile);
    // check that startIdx is >-1 and that stopIdx is less than imu length
    if(startIdx<0){
        std::cerr<<"startIdx must be positive integer"<<std::endl;
        return 4;
    }
    if(stopIdx>(ImuMap.begin()->second).length()){
        std::cerr<<"stopIdx out of bounds of IMU length"<<std::endl;
        return 5;
    }
    // now cut imu map
    std::map<std::string,imu> ImuMapCut=datapkgr::cutImuMapByIdx(ImuMap,startIdx,stopIdx);
    // write imu map to output h5 file
    datapkgr::writeImuToApdmOpalH5File(ImuMapCut,outputFile.string());
    return 0;
}
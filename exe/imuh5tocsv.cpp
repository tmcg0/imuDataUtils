#include <iostream>
#include <boost/filesystem.hpp>
#include <imu.h>
#include <datapkgr.h>

// usage: ./imuh5tocsv path/to/h5/file.h5 file/to/save/to.csv

int main(int argc, char** argv){
    // check that exactly 3 arguments are entered
    if(argc!=3){
        std::cerr<<"enter 2 arguments! usage: path/to/file.h5 file/to/save.csv"<<std::endl;
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
    // check that second argument is an csv file and its parent directory exists (create if not)
    boost::filesystem::path outputFile(argv[2]);
    boost::filesystem::create_directory(outputFile.parent_path());
    std::string outputFileExt=boost::filesystem::extension(outputFile);
    if(".csv"!=outputFileExt){
        std::cerr<<"output file "<<outputFile<<" does not represent a valid .h5 file"<<std::endl;
        return 3;
    }
    // construct imu map from input file
    std::map<std::string,imu> ImuMap=imu::getImuMapFromDataFile(inputFile);
    datapkgr::apdmh5ToCsv(inputFile, outputFile.string());
    return 0;
}
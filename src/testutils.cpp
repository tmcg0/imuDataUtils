
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <math.h>
#include "testutils.h"

namespace testutils
{

    std::string getTestDataDir(){
        // assumes test data is at bioslamroot/test/data
        boost::filesystem::path testdatadir=boost::filesystem::path(getProjectRootDir()+"/test/data");
        return testdatadir.string();
    }

    std::string getProjectRootDir(){
        // iterates down to current path until it finds directory named 'bioslam'
        boost::filesystem::path bioslamroot;
        bool foundBioslamRoot=false;
        for(auto& part : boost::filesystem::current_path()) {
            bioslamroot /= part.c_str();
            if(std::strcmp(part.c_str(),"imuUtils")==0){ // you found it. exit.
                foundBioslamRoot=true;
                break;
            }
        }
        if(!foundBioslamRoot){
            std::cout<<"EXCEPTION: could not bioslam root directory! (searched for directory in tree named 'bioslam')"<<std::endl;
        }
        return bioslamroot.string();
    }

    std::string getTestDataFile(std::string filename){
        // before returning string to full path of file, checks to make sure it exists
        std::string proposedFile=getTestDataDir()+"/"+filename;
        if(boost::filesystem::exists(proposedFile)){//file exists, all is well
            return proposedFile;
        }else{ // bad
            std::cerr<<"ERROR: this ain't a file, yo. ("<<filename<<")"<<std::endl;
            return "ERROR";
        }
    }

    std::string getDropboxRoot(){
        // return dropbox root directory. also checks if directory exists (this is poor man's check for machine ID--only Tim can run the experimental data tests)
        boost::filesystem::path dropboxRoot="/home/tmcgrath/Dropbox (MIT)/";
        if(boost::filesystem::is_directory(dropboxRoot)){ // you found a valid dropbox root
            return dropboxRoot.string();
        }else{ // couldn't find this path. are you sure you're on one of Tim's machines?
            std::cout<<"ERROR: could not find dropbox root. Are you sure you're on one of Tim's machines?"<<std::endl;
            return "";
        }
    }


    double findVectorAverage(std::vector<double> vec, int startIdx, int endIdx){
        double avg=0.0;
        for(int i=startIdx; i<=endIdx; i++){
            avg+=vec[i];
        }
        avg/=(endIdx-startIdx);
        return avg;
    }
} // namespace
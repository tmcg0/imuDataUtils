#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include "imuDataTestUtils.h"

namespace imuDataTestUtils
{

    std::string getTestDataDir(){
        // assumes test data is at bioslamroot/test/data
        boost::filesystem::path testdatadir=boost::filesystem::path(getProjectRootDir()+"/test/data");
        return testdatadir.string();
    }

    std::string getProjectRootDir(){
        // iterates down to current path until it finds directory named 'bioslam'
        boost::filesystem::path projectroot;
        bool foundRoot=false;
        for(auto& part : boost::filesystem::current_path()) {
            projectroot /= part.c_str();
            if(std::strcmp(part.c_str(),"imuDataUtils")==0){ // you found it. exit.
                foundRoot=true;
                break;
            }
        }
        if(!foundRoot){
            std::cout<<"EXCEPTION: could not find root directory! (searched for directory in tree named 'imuDataUtils')"<<std::endl;
        }
        return projectroot.string();
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

    void verifyDataPackaging(const imu& testImu, bool verbose){
        // verify that the data is good. all same size.
        uint testLength=testImu.length(); // let's hope they're all this size
        if(testImu.ax.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.ay.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.az.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.gx.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.gy.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.gz.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.mx.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.my.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.mz.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.relTimeSec.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        // now test quaternion
        if(testImu.qs.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.qx.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.qy.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(testImu.qz.size()!=testLength){throw std::runtime_error("inconsistent size.");}
        if(verbose){
            std::cout<<"everything looks good with this imu."<<std::endl;
        }
    }

} // namespace
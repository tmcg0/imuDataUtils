// test functionality of v5 APDM .h5 files

#include <testutils.h>
#include <iostream>
#include <datapkgr.h>
#include "imu.h"

int main(){
    std::map<std::string,imu> ImuMap=imu::getImuMapFromDataFile(testutils::getTestDataFile("apdm_v5_multiple_sensors.h5"));
    imu::printLabelsInFile(testutils::getTestDataFile("apdm_v5_multiple_sensors.h5"));
    imu imu1=ImuMap["Sacrum"];
    std::cout<<"imu1 gx[5]="<<imu1.gx[5]<<std::endl;
    imu imu1copy=imu1; // this syntax is a standard copy
    imu1.gx[5]=999.999;
    std::cout<<"imu1 gx[5]="<<imu1.gx[5]<<std::endl;
    std::cout<<"imu1copy gx[5]="<<imu1copy.gx[5]<<std::endl;

    imu cutImu=imu1.cutImuByIdx(10, 20);
    std::cout<<"cutImu gx[5]="<<cutImu.gx[5]<<std::endl;
    std::cout<<"length of cut imu="<<cutImu.length()<<std::endl;

    // write chopped imu
    datapkgr::writeImuToApdmOpalH5File(cutImu,"/home/tmcgrath/imuUtils/test/output/testfile.h5");
    return 0;
}
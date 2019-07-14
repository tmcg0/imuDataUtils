// test functionality of v5 APDM .h5 files

#include <imuDataTestUtils.h>
#include <iostream>
#include <datapkgr.h>
#include "imu.h"

// todo: add functionality for writing entire map of imus to file
int main(){
    std::map<std::string,imu> ImuMap=imu::getImuMapFromDataFile(imuDataTestUtils::getTestDataFile("apdm_v5_multiple_sensors.h5"));
    imu::printLabelsInFile(imuDataTestUtils::getTestDataFile("apdm_v5_multiple_sensors.h5"));
    imu imu1=ImuMap["Sacrum"];
    imu cutImu=imu1.cutImuByIdx(10, 20);
    // write chopped imu
    std::string imuFileOut=imuDataTestUtils::getProjectRootDir()+"/test/output/testfile.h5";
    datapkgr::writeImuToApdmOpalH5File(cutImu,imuFileOut);
    // read chopped imu back
    std::map<std::string,imu> ImuMap2=imu::getImuMapFromDataFile(imuFileOut);
    // NOW: try to write entire imu map back to file
    std::string imuMapFileOut=imuDataTestUtils::getProjectRootDir()+"/test/output/testfilemap.h5";
    datapkgr::writeImuToApdmOpalH5File(ImuMap,imuMapFileOut);
    // read chopped imu back
    std::map<std::string,imu> ImuMap3=imu::getImuMapFromDataFile(imuMapFileOut);
    return 0;
}
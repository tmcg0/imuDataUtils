// test the imu class and its functions

#include <imuDataTestUtils.h>
#include "imu.h"

int main(){
    std::map<std::string,imu> ImuMap=imu::getImuMapFromDataFile(imuDataTestUtils::getTestDataFile("apdm_v5_multiple_sensors.h5"));
    imu::printLabelsInFile(imuDataTestUtils::getTestDataFile("apdm_v5_multiple_sensors.h5"));
    imu imu1=ImuMap["Sacrum"];

    imuDataTestUtils::verifyDataPackaging(imu1,true);
    return 0;
}
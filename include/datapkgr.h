// a namespace to handle data packing and unpacking (file io) for IMU + biomechanics data

#pragma once

#include <string>
#include <vector>
#include "highfive/H5DataSet.hpp"
#include "imu.h" // include imu--then you can use datapkgr as an extended constructor for imu

namespace h5=HighFive;

namespace datapkgr
{
    imudata readSingleImuDataFromApdmOpalH5FileByLabel(std::string filestr, std::string label);
    std::vector<std::string> getAllImuLabelsInDataFile(std::string filestr);
    void writeImuToApdmOpalH5File();
    bool is_apdm_h5_version5(std::string filestr);
    std::vector<std::vector<double>> get_2d_data_from_dataset(h5::DataSet ds);
    std::vector<double> get_1d_data_from_dataset(h5::DataSet ds);
}

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
    int apdmh5v5ToCsv(const std::string& apdmH5File, const std::string& csvFileToWrite);
    void writeImuToApdmOpalH5File(const imu& imuToWrite, const std::string& h5Filename);
    bool is_apdm_h5_version5(std::string filestr);
    std::vector<std::vector<double>> get_2d_data_from_dataset(const h5::DataSet& ds);
    std::vector<double> get_1d_data_from_dataset(const h5::DataSet& ds);
    int write_1d_data_to_dataset(const h5::DataSet& ds, std::vector<double>);
}

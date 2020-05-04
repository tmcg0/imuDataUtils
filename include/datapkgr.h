// a namespace to handle data packing and unpacking (file io) for IMU + biomechanics data

#pragma once

#include <string>
#include <vector>
#include "highfive/H5DataSet.hpp"
#include "imu.h" // include imu--then you can use datapkgr as an extended constructor for imu

namespace h5=HighFive;

namespace datapkgr{
    imudata readSingleImuDataFromApdmOpalH5FileByLabel(const std::string& filestr, const std::string& label);
    std::vector<std::string> getAllImuLabelsInDataFile(const std::string& filestr);
    int apdmh5ToCsv(const std::string& apdmH5File, const std::string& csvFileToWrite);
    int writeImuMapToCsv(const std::map<std::string,imu>& imuMapToWrite, const std::string& csvFileToWrite);
    std::map<std::string,imu> cutImuMapByIdx(std::map<std::string,imu>& ImuMap, uint startIdx, uint stopIdx);
    void writeImuToApdmOpalH5File(const imu& imuToWrite, const std::string& h5Filename);
    void writeImuToApdmOpalH5File(const std::map<std::string,imu>& imuMapToWrite, const std::string& h5Filename);
    bool is_apdm_h5_version5(std::string filestr);
    std::vector<std::vector<double>> get_2d_data_from_dataset(const h5::DataSet& ds);
    std::vector<double> get_1d_data_from_dataset(const h5::DataSet& ds);
    int write_1d_data_to_dataset(const h5::DataSet& ds, std::vector<double>);
    std::vector<std::vector<double>> makeNestedVector(const std::vector<double>& data1, const std::vector<double>& data2, const std::vector<double>& data3);
    int apdmCaseIdStringToInt(const std::string &caseId);
    template<typename T>
    static int getNearestIdxFromVector(const std::vector<T>& myvec, const T& queryPt){
        std::vector<T> newvec(myvec.size());
        double minDiff=9e9;
        int minIdx=0;
        for(int i=0;i<myvec.size();i++){
            newvec[i]=abs(myvec[i]-queryPt);
            if(newvec[i]<minDiff){
                minDiff=newvec[i];
                minIdx=i;
            }
        }
        return minIdx;
    }
}



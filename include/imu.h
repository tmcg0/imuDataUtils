#pragma once

#include <string>
#include <vector>
#include <map>

class imu {
public:
    // constructors
    imu()=default; // empty constructor
    imu(std::string filePath, std::string labelName); // construct a single imu from a label name in .h5 file
    std::vector<double> gx, gy, gz, ax, ay, az, mx, my, mz, qs, qx, qy, qz; // IMU sensor data arrays
    std::vector<double> relTimeSec; // time of measurements, relative to zero in seconds
    std::vector<double> unixTimeUtc; // unix time stamp in seconds, utc
    int id; // unique id integer. in apdm sensors that looks like XI-000###
    std::vector<std::vector<double>> quaternion() const; // retrieve quaternion as vector<vector<double>>
    std::string label; // a label to uniquely id an imu in a set
    void print_sensor_maxmin();
    unsigned long length() const;
    imu cutImuByIdx(uint startIdx, uint stopIdx);
    imu cutImuByTime(double startTime, double stopTime);
    double getDeltaT() const;

    // static member functions
    static std::map<std::string,imu> getImuMapFromDataFile(const std::string& filestr);
    static void printLabelsInFile(const std::string& datafilestr);
    static bool isUnixTimeSec(unsigned long time); // is this a valid unix time in seconds?

}; // end classdef

template<typename T>
std::vector<T> slice(std::vector<T> const &v, int m, int n){
    auto first = v.cbegin() + m;
    auto last = v.cbegin() + n + 1;
    std::vector<T> vec(first, last);
    return vec;
}

struct imudata{
    // this is a simple struct to hold pure imu data for a *single* imu!
    std::vector<double> ax, ay, az, gx, gy, gz, mx, my, mz, relTimeSec, pressure, temperature;
    std::vector<double> qs, qx, qy, qz;
    std::vector<double> unixTimeUtcMicrosec; // coerce these into a double if necessary (often read from a unit timestamp)
    std::string label;
    int id;
};

#pragma once

#include <string>
#include <vector>
#include <map>

class imu {
public:
    // constructors
    imu()=default; // empty constructor
    imu(std::string filePath, int readIdx); // construct from .h5 file
    imu(std::string filePath, std::string labelName); // construct a single imu from a label name in .h5 file
    std::vector<double> gx, gy, gz, ax, ay, az, mx, my, mz, qs, qx, qy, qz; // IMU sensor data arrays
    std::vector<double> relTimeSec; // time of measurements, relative to zero in seconds
    std::vector<double> unixTimeUTC; // unix time stamp in seconds, utc
    std::vector<std::vector<double>> quaternion(); // turn quaternion into vector<vector<double>>
    std::string label; // a label to uniquely id an imu in a set
    void print_sensor_maxmin();
    unsigned long length() const;
    imu cutImuByIdx(int startIdx, int stopIdx);
    imu cutImuByTime(double startTime, double stopTime);
    // --- public get methods --- //
    double getDeltaT();
    // --- public set methods --- //

    // static member functions
    static std::map<std::string,imu> getImuMapFromDataFile(std::string filestr);
    static void printLabelsInFile(std::string datafilestr);
}; // end classdef

struct imudata{
    // this is a simple struct to hold pure imu data for a *single* imu!
    std::vector<double> ax, ay, az, gx, gy, gz, mx, my, mz, measTime, pressure, temperature;
    std::vector<double> unixTime;
    std::string label;
};

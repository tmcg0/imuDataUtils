#pragma once

#include <string>
#include <gtsam/geometry/Rot3.h>
#include <map>

class imu {
public:
    // constructors
    imu()=default; // empty constructor
    imu(std::vector<double> time, std::vector<double> gx, std::vector<double> gy, std::vector<double> gz, std::vector<double> ax, std::vector<double> ay, std::vector<double> az, std::vector<double> mx, std::vector<double> my, std::vector<double> mz); //construct from data arrays
    imu(std::string filePath, int readIdx); // construct from .h5 file
    imu(std::string filePath, std::string labelName); // construct a single imu from a label name in .h5 file
    std::string qsrc;
    std::vector<double> gx, gy, gz, ax, ay, az, mx, my, mz; // IMU sensor data arrays
    std::vector<double> measTime; // time of measurements
    std::vector<double> estTime; // time of external estimation procedure
    std::vector<std::vector<double>> qAPDM; // orientation quaternion according to APDM (if relevant)
    double m_deltaT;
    std::string label; // a label to uniquely id an imu in a set
    std::vector<double> B_NWU {30.5, 14.91, -56.3}; // global NWU magnetic definition
    gtsam::Rot3 q_method(Eigen::Vector3d accelBody, Eigen::Vector3d magBody, Eigen::Vector3d accelNWU, Eigen::Vector3d magNWU, double wa, double wm);
    void print_sensor_maxmin();
    unsigned long length() const;
    void relativeAngularVelocity(imu imuB);
    gtsam::Vector3 accel_Vector3(int idx);
    gtsam::Vector3 mags_Vector3(int idx);
    gtsam::Vector3 gyros_Vector3(int idx);
    imu cutImuByIdx(int startIdx, int stopIdx);
    imu cutImuByTime(double startTime, double stopTime);
    // --- public get methods --- //
    std::vector<gtsam::Rot3> getOrientation();
    std::vector<gtsam::Rot3> getDefaultOrientation();
    // --- public set methods --- //
    void setOrientation(std::vector<gtsam::Rot3> r);

    // static member functions
    static std::map<std::string,imu> getImuMapFromDataFile(std::string filestr);
    static void printLabelsInFile(std::string datafilestr);
private:
    std::vector<gtsam::Rot3> orientation; // q[NWU->local]
    std::vector<gtsam::Rot3> defaultOrientation; // q[NWU->local]. default orientation would come from a built-in source, like the manufacturer's filter
    // --- private set methods --- //
    void setDefaultOrientation(std::vector<gtsam::Rot3> r); // I don't want end user to set the default orientation. must come from source.
}; // end classdef

struct imudata{
    // this is a simple struct to hold pure imu data for a *single* imu!
    std::vector<double> ax, ay, az, gx, gy, gz, mx, my, mz, measTime, pressure, temperature;
    std::vector<double> unixTime;
    std::vector<gtsam::Rot3> orientation; // orientation vector
    std::string label;
};

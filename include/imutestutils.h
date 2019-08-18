// A set of utilities to aid in testing... not required to build the general library

#pragma once

#include "imu.h"

namespace imutestutils
{
    std::string getTestDataDir();
    std::string getProjectRootDir();
    std::string getTestDataFile(std::string filename);
    double findVectorAverage(std::vector<double> vec, int startIdx, int endIdx);
    std::string getDropboxRoot();
}
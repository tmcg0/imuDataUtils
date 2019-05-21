# imuDataPkgr
IMU File I/O

## General data and library
##### A general C++ class and functions for constructing and holding IMU data
Class ```imu.h``` represents a generic class to hold IMU data. It may be constructed directly or a map may be created to map the IMU to its label. ```datapkgr``` namepspace handles this. It also provides an extended contructor for ```imu``` to conveniently construct that map from an APDM \*.h5 file which you get from the Opal sensors.


## Executables
##### In the /exe/ folder you'll find the source code for 3 executables which are built by CMake:
1) cutImuH5ByIdxs: cut an h5 file of IMUs down to certain indexes in the data. 

usage: ``` ./cutImuH5ByIdxs path/to/file.h5 file/to/save.h5 (int)StartIdx (int)StopIdx ```

2) cutImuH5ByUnixTimestamps: same as above but cut down by nearest unix time stamps in the data (microseconds UTC)

usage: ``` ./cutImuH5ByUnixTimestamps path/to/file.h5 file/to/save.h5 (int)StartTime (int)StopTime ```

3) imuh5tocsv: convert an h5 file of IMU data to a csv file

usage: ```./imuh5tocsv path/to/h5/file.h5 file/to/save/to.csv```

## Dependencies
- Boost::Filesystem

## Installation
None. Just download source, check dependenices, and build using CMake.

## ⚠️ this library is no longer actively maintained, but left for posterirty of users who need convenience wrappers to the [APDM Opal](https://apdm.com/wearable-sensors/) v1 and v2 hardware model's [HDF5 file specification](https://share.apdm.com/documentation/DevelopersGuide.pdf)

## imuDataUtils

##### A general C++ library for constructing and holding IMU data
Class ```imu``` represents a generic class to hold IMU data. It may be constructed directly or a map may be created to map the IMU to its label. ```datapkgr``` namepspace handles this. It also provides an extended contructor for ```imu``` to conveniently construct that map from an IMU file.

**Note**: imuDataUtils does not perform estimation of any sort on IMU data. Its purpose is to simply read, write, and hold IMU data across different sources. It is intended to be a useful subcomponent of larger estimation frameworks.

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
- installed [HighFive library](https://github.com/BlueBrain/HighFive)
- hdf5 libraries (Ubuntu: `sudo apt-get install libhdf5-serial-dev`)

## Installation
Built using CMake
From root of repo:
```
- mkdir build
- cd build
- cmake ..
- make install # (may require root privileges)
- make test # (optional, runs unit tests)
```

## Supported IMU file types
See [supported IMU file formats](https://github.com/tmcg0/imuDataUtils/wiki/Supported-IMU-file-formats) for more info.
- APDM .h5 files (version 5)

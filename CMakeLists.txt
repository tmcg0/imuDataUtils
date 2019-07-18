message(STATUS "+++++++++++++++++++   Beginning CMake IMU utilities   ++++++++++++++++++++")
cmake_minimum_required(VERSION 3.5)
project(imuDataUtils VERSION 0.0 LANGUAGES CXX)
set(PROJECT_DESCRIPTION "File I/O for various IMUs") # for compatibility with CMake 3.8.2
set(CMAKE_CXX_STANDARD 14)

### ----- User options ----- ###
option(BUILD_TESTS "Should I build tests?" ON)
option(BUILD_CMD_LINE_TOOL "Build the Linux command line tool?" ON)
### ------------------------ ###

# ++++++++++++++++++ handle CMake flags ++++++++++++++++++++ #
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release) # by default build as 'Release'
endif()
# set(CMAKE_CXX_FLAGS "-Wall") # optional warning outputs?
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
message(STATUS "Building as version type: ${CMAKE_BUILD_TYPE} ")
# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #

# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
# ++++++++++++++++++ find and use all dependent libraries of imuDataUtils +++++++++++++++++++++ #
set(IMUDATAUTILSLIBS) # variables list for adding libraries to
# --- Require Boost ---
find_package(Boost 1.58 COMPONENTS filesystem REQUIRED )
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND IMUDATAUTILSLIBS  ${Boost_LIBRARIES})
# --- Require HDF5 ---
find_package(HDF5 COMPONENTS CXX REQUIRED) # have to find CXX component first for some reason
add_definitions(${HDF5_CXX_DEFINITIONS})
include_directories(${HDF5_CXX_INCLUDE_DIR})
list(APPEND IMUDATAUTILSLIBS ${HDF5_CXX_LIBRARIES})
find_package(HDF5 REQUIRED)
add_definitions(${HDF5_DEFINITIONS})
include_directories(${HDF5_INCLUDE_DIR})
list(APPEND IMUDATAUTILSLIBS ${HDF5_LIBRARIES})
# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #

# make all includes and sources
message(STATUS "proj source dir is ${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include") # headers
file(GLOB SOURCES_IMUDATAUTILS "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/include/*.h")
message(STATUS "sources are ${SOURCES_IMUDATAUTILS}")
# create and link library
add_library(imuDataUtils SHARED ${SOURCES_IMUDATAUTILS})
target_link_libraries(imuDataUtils ${IMUDATAUTILSLIBS})
message(STATUS "Libs that we have for imuDataUtils are ${IMUDATAUTILSLIBS} ")

# make tests for CTest environment
if (BUILD_TESTS)
    enable_testing()
    # make unit tests
    add_executable(apdm_v5_unit_test test/test_apdm_v5.cpp)
    target_link_libraries(apdm_v5_unit_test imuutils ${IMUUTILSLIBS})
    add_executable(imuClassUnitTest test/imuClassUnitTest.cpp)
    target_link_libraries(imuClassUnitTest imuutils ${IMUUTILSLIBS})
    # add test runners
    add_test(run_apdm_v5_unit_test apdm_v5_unit_test)
    add_test(run_imuClassUnitTest imuClassUnitTest)
endif()

if(BUILD_CMD_LINE_TOOL)
    # todo: building command line tool for linux. tool should have following functionality:
    # - chop an h5 file into a smaller h5 file by index and timestamp
    # - convert h5 file to csv
    add_executable(cutImuH5ByIdxs exe/cutImuH5ByIdxs.cpp)
    target_link_libraries(cutImuH5ByIdxs imuDataUtils ${IMUDATAUTILSLIBS})
    add_executable(cutImuH5ByUnixTimestamps exe/cutImuH5ByUnixTimestamps.cpp)
    target_link_libraries(cutImuH5ByUnixTimestamps imuDataUtils ${IMUDATAUTILSLIBS})
    add_executable(imuh5tocsv exe/imuh5tocsv.cpp)
    target_link_libraries(imuh5tocsv imuDataUtils ${IMUDATAUTILSLIBS})
endif()

message(STATUS "+++++++++++++++++++   End CMake   ++++++++++++++++++++")

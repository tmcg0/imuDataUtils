message(STATUS "+++++++++++++++++++   Beginning CMake IMU utilities   ++++++++++++++++++++")
cmake_minimum_required(VERSION 3.5)
project(imuUtils VERSION 0.0 LANGUAGES CXX)
set(PROJECT_DESCRIPTION "File I/O for various IMUs") # for compatibility with CMake 3.8.2
set(CMAKE_CXX_STANDARD 14)

### ----- User options ----- ###
option(BUILD_TESTS "Should I build tests?" ON)
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
# ++++++++++++++++++ find and use all dependent libraries of imuUtils +++++++++++++++++++++ #
set(IMUUTILSLIBS) # variables list for adding libraries to
# --- Require Eigen ---
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
IF(ON_SUPERCLOUD)
    message(STATUS "including Eigen at: /home/gridsan/tmcgrath/installs/include/eigen3")
    include_directories("/home/gridsan/tmcgrath/installs/include/eigen3/")
else()
    message(STATUS "including Eigen at: /usr/include/eigen3")
    include_directories("/usr/include/eigen3/")
endif()
if (TARGET Eigen3::Eigen)
    message(STATUS "found Eigen at: ${EIGEN3_DIR}")
endif (TARGET Eigen3::Eigen)
# --- include MKL headers --- # (optional, only required by GTSAM)
IF(GTSAM_USE_MKL)
    IF(IS_DIRECTORY "/opt/intel/pstudio/mkl/include") # ew: hardcoding directories. switch to using a module for this.
        set(MKL_INCLUDE_DIR "/opt/intel/pstudio/mkl/include")
    endif()
    IF(IS_DIRECTORY "/home/tmcgrath/intel/mkl/include") # ew: hardcoding directories. switch to using a module for this.
        set(MKL_INCLUDE_DIR "/home/tmcgrath/intel/mkl/include")
    endif()
    message(STATUS "including MKL headers from ${MKL_INCLUDE_DIR}")
    include_directories(${MKL_INCLUDE_DIR})
    set(MKL_LIBRARIES)
    list(APPEND MKL_LIBRARIES "/home/tmcgrath/intel/mkl/lib/intel64/libmkl_intel_lp64.so")
    list(APPEND MKL_LIBRARIES "/home/tmcgrath/intel/mkl/lib/intel64/libmkl_gnu_thread.so")
    list(APPEND MKL_LIBRARIES "/home/tmcgrath/intel/mkl/lib/intel64/libmkl_core.so")
    list(APPEND BIOSLAMLIBS ${MKL_LIBRARIES})
endif()
# --- Require Boost ---
find_package(Boost 1.58 COMPONENTS filesystem REQUIRED )
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND BIOSLAMLIBS  ${Boost_LIBRARIES})
# --- Require Python 2.7 for using matplotlib-cpp ---
#     (you need from sudo apt install: [python2.7, python-numpy, python-matplotlib])
find_package(PythonLibs 2.7)
include_directories(${PYTHON_INCLUDE_DIRS})
list(APPEND BIOSLAMLIBS ${PYTHON_LIBRARIES})
# --- Require HDF5 ---
find_package(HDF5 COMPONENTS CXX REQUIRED) # have to find CXX component first for some reason
add_definitions(${HDF5_CXX_DEFINITIONS})
include_directories(${HDF5_CXX_INCLUDE_DIR})
list(APPEND BIOSLAMLIBS ${HDF5_CXX_LIBRARIES})
find_package(HDF5 REQUIRED)
add_definitions(${HDF5_DEFINITIONS})
include_directories(${HDF5_INCLUDE_DIR})
list(APPEND BIOSLAMLIBS ${HDF5_LIBRARIES})
# --- Require GTSAM ---
find_package(GTSAM REQUIRED) # this command is made from FindGTSAM.cmake in cmake/ folder
include_directories(${GTSAM_INCLUDE_DIRS})
list(APPEND BIOSLAMLIBS ${GTSAM_LIBRARIES})
# --- optionally include TBB if GTSAM wants it ---
find_package(TBB)
message(STATUS "setting tbb include dirs to: ${TBB_INCLUDE_DIRS}")
include_directories(${TBB_INCLUDE_DIRS})
message(STATUS "tbb libraries are:  ${TBB_LIBRARIES}}")
list(APPEND BIOSLAMLIBS ${TBB_LIBRARIES})
# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #

# make all includes and sources
message(STATUS "proj source dir is ${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/include") # bioslam headers
include_directories("${PROJECT_SOURCE_DIR}/include/factors") # bioslam headers for factors
file(GLOB SOURCES_BIOSLAM "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/factors/*.cpp" "${PROJECT_SOURCE_DIR}/include/*.h" "${PROJECT_SOURCE_DIR}/include/factors/*.h")
message(STATUS "bioslam sources are ${SOURCES_BIOSLAM}")
# create and link library
add_library(bioslam SHARED ${SOURCES_BIOSLAM})
target_link_libraries(bioslam ${BIOSLAMLIBS})
message(STATUS "Libs that we have for bioslam are ${BIOSLAMLIBS} ")

# make tests for CTest environment
if (BUILD_TESTS)
    enable_testing()
    # make unit tests
    add_executable(IMU_SLAM_UNIT_TEST test/unit/testSingleImuEstimation.cpp)
    target_link_libraries(IMU_SLAM_UNIT_TEST bioslam ${BIOSLAMLIBS})
    add_executable(H5_UNIT_TEST test/unit/testh5.cpp)
    target_link_libraries(H5_UNIT_TEST bioslam ${BIOSLAMLIBS})
    add_executable(ROT_UNIT_TEST test/unit/testGtsamRots.cpp)
    target_link_libraries(ROT_UNIT_TEST bioslam ${BIOSLAMLIBS})
    add_executable(KNEE_SLAM_UNIT_TEST test/unit/testKneeEstimation.cpp)
    target_link_libraries(KNEE_SLAM_UNIT_TEST bioslam ${BIOSLAMLIBS})
    add_executable(LEG_SLAM_UNIT_TEST test/unit/testLegEstimation.cpp)
    target_link_libraries(LEG_SLAM_UNIT_TEST bioslam ${BIOSLAMLIBS})
    add_executable(PCA_UNIT_TEST test/unit/testPCA.cpp)
    target_link_libraries(PCA_UNIT_TEST bioslam ${BIOSLAMLIBS})
    add_executable(test_relAngVelPcaAxisEstimation test/unit/testRelAngVelPcaEstimation.cpp)
    target_link_libraries(test_relAngVelPcaAxisEstimation bioslam ${BIOSLAMLIBS})
    add_executable(MAGFACTOR_UNIT_TEST test/unit/testMagFactor.cpp)
    target_link_libraries(MAGFACTOR_UNIT_TEST bioslam ${BIOSLAMLIBS})
    add_executable(KNEEFACTOR_UNIT_TEST test/unit/testKneeAxisFactor.cpp)
    target_link_libraries(KNEEFACTOR_UNIT_TEST bioslam ${BIOSLAMLIBS})
    add_executable(imuBiasModelHandler_unit_test test/unit/testImuBiasModelHandler.cpp)
    target_link_libraries(imuBiasModelHandler_unit_test bioslam ${BIOSLAMLIBS})
    add_executable(benchmarkRobustOptimize test/benchmark/benchmarkRobustOptimize.cpp)
    target_link_libraries(benchmarkRobustOptimize bioslam ${BIOSLAMLIBS})
    add_executable(testImuFactors test/unit/testImuFactors.cpp)
    target_link_libraries(testImuFactors bioslam ${BIOSLAMLIBS})
    add_executable(testBruteForceKneeAngleCalc test/unit/testBruteForceKneeAngleCalc.cpp)
    target_link_libraries(testBruteForceKneeAngleCalc bioslam ${BIOSLAMLIBS})

    # make solution tests (will add this functionality later. stick to unit tests for now)
    # add_executable(IMU_SLAM_SOLUTION_TEST test/solution/testImuEstimationSolution.cpp)
    # target_link_libraries(IMU_SLAM_SOLUTION_TEST bioslam ${BIOSLAMLIBS})

    # add tests
    add_test(RUN_IMU_SLAM_UNIT_TEST IMU_SLAM_UNIT_TEST)
    add_test(RUN_H5_UNIT_TEST H5_UNIT_TEST)
    add_test(RUN_ROT_UNIT_TEST ROT_UNIT_TEST)
    add_test(RUN_KNEE_SLAM_UNIT_TEST KNEE_SLAM_UNIT_TEST)
    add_test(RUN_LEG_SLAM_UNIT_TEST LEG_SLAM_UNIT_TEST)
    add_test(RUN_PCA_UNIT_TEST PCA_UNIT_TEST)
    #add_test(RUN_IMU_SLAM_SOLUTION_TEST bin/IMU_SLAM_SOLUTION_TEST)
    add_test(RUN_MAGFACTOR_UNIT_TEST MAGFACTOR_UNIT_TEST)
    add_test(RUN_imuBiasModelHandler_unit_test imuBiasModelHandler_unit_test)
endif()

#=====================================================#
#==================== DEBUG ONLY =====================#
# make Tim's non-test binaries for just debugging software
add_executable(debugMagFactorEstimation exe/debugMagFactorEstimation.cpp)
target_link_libraries(debugMagFactorEstimation bioslam ${BIOSLAMLIBS})
add_executable(tugExperiment exe/tugExperiment/testTugExperiment.cpp) # tug experiment from last summer
target_link_libraries(tugExperiment bioslam ${BIOSLAMLIBS})
add_executable(treadmillPilot exe/treadmillPilot/treadmillPilot.cpp) # treadmill pilot data
target_link_libraries(treadmillPilot bioslam ${BIOSLAMLIBS})
add_executable(debugKneeEstimation exe/debugKneeEstimation.cpp)
target_link_libraries(debugKneeEstimation bioslam ${BIOSLAMLIBS})
#=====================================================#
#=====================================================#

message(STATUS "+++++++++++++++++++   End CMake of bioslam   ++++++++++++++++++++")

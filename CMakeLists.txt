# Introduce variables:
# * CMAKE_INSTALL_LIBDIR
# * CMAKE_INSTALL_BINDIR
# * CMAKE_INSTALL_INCLUDEDIR
include(GNUInstallDirs)

message(STATUS "+++++++++++++++++++   Beginning CMake IMU utilities   ++++++++++++++++++++")
cmake_minimum_required(VERSION 3.5)
project(imuDataUtils VERSION 0.0 LANGUAGES CXX)
set(PROJECT_DESCRIPTION "File I/O for various IMUs") # for compatibility with CMake 3.8.2
set(CMAKE_CXX_STANDARD 14)

### ----- User options ----- ###
option(IMUDATAUTILS_BUILD_TESTS "Should I build tests?" ON)
option(IMUDATAUTILS_BUILD_CMD_LINE_TOOL "Build the Linux command line tool?" ON)
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

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
    set(DEF_INSTALL_CMAKE_DIR cmake)
else()
    set(DEF_INSTALL_CMAKE_DIR lib/cmake/imuDataUtils)
endif()
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files  ")

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
# --- Require HighFive --- #
find_package(HighFive REQUIRED)
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
target_link_libraries(imuDataUtils HighFive)
message(STATUS "Libs that we have for imuDataUtils are ${IMUDATAUTILSLIBS} ")

# make tests for CTest environment
if (IMUDATAUTILS_BUILD_TESTS)
    enable_testing()
    # make unit tests
    add_executable(apdm_v5_unit_test test/test_apdm_v5.cpp)
    target_link_libraries(apdm_v5_unit_test imuDataUtils ${IMUUTILSLIBS})
    add_executable(imuClassUnitTest test/imuClassUnitTest.cpp)
    target_link_libraries(imuClassUnitTest imuDataUtils ${IMUUTILSLIBS})
    # add test runners
    add_test(run_apdm_v5_unit_test apdm_v5_unit_test)
    add_test(run_imuClassUnitTest imuClassUnitTest)
endif()

if(IMUDATAUTILS_BUILD_CMD_LINE_TOOL)
    # todo: building command line tool for linux. tool should have following functionality:
    # - chop an h5 file into a smaller h5 file by index and timestamp
    # - convert h5 file to csv
    add_executable(cutImuH5ByIdxs exe/cutImuH5ByIdxs.cpp)
    target_link_libraries(cutImuH5ByIdxs imuDataUtils ${IMUDATAUTILSLIBS})
    add_executable(cutImuH5ByUnixTimestamps exe/cutImuH5ByUnixTimestamps.cpp)
    target_link_libraries(cutImuH5ByUnixTimestamps imuDataUtils ${IMUDATAUTILSLIBS})
    add_executable(imuh5tocsv exe/imuh5tocsv.cpp)
    target_link_libraries(imuh5tocsv imuDataUtils ${IMUDATAUTILSLIBS})
    add_executable(validateImuFile exe/validateImuFile.cpp)
    target_link_libraries(validateImuFile imuDataUtils ${IMUDATAUTILSLIBS})
endif()


# ========= handle installing and exporting ======== #
file(RELATIVE_PATH CONF_REL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/${DEF_INSTALL_CMAKE_DIR}" "${CMAKE_INSTALL_PREFIX}/include")
file(RELATIVE_PATH CONF_REL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/${DEF_INSTALL_CMAKE_DIR}" "${CMAKE_INSTALL_PREFIX}/lib")

# Add all targets to the build-tree export set
export(TARGETS imuDataUtils FILE "${PROJECT_BINARY_DIR}/ImuDataUtilsTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE imuDataUtils)

include(CMakePackageConfigHelpers)
message(STATUS "cmake current binary dir = ${CMAKE_CURRENT_BINARY_DIR}")
write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/ImuDataUtilsConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

# Create ImuDataUtilsConfig.cmake with extra info from ImuDataUtilsConfig.cmake.in
# This file is necessary to find_package the library imuDataUtils.
set(INSTALL_CONFIGDIR lib/cmake/imuDataUtils)
configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/cmake/ImuDataUtilsConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/ImuDataUtilsConfig.cmake
        INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# install the library
message(STATUS "library destination = ${CMAKE_INSTALL_LIBDIR}")
install(TARGETS imuDataUtils
        EXPORT imuDataUtils-export
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT shlib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin)

# install the header files
message(STATUS "CMAKE_INSTALL_FULL_INCLUDEDIR = ${CMAKE_INSTALL_FULL_INCLUDEDIR}")
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include/ DESTINATION include/imuDataUtils FILES_MATCHING PATTERN "*.h")

# Install the ImuDataUtilsConfig.cmake, ImuDataUtilsConfigVersion.cmake, and ImuDataUtilsTargets.cmake files
install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ImuDataUtilsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ImuDataUtilsConfigVersion.cmake"
        DESTINATION "${INSTALL_CMAKE_DIR}")
message(STATUS "cmake install dir: ${INSTALL_CMAKE_DIR} ")

# install the exported ImuDataUtilsTargets.cmake
install(EXPORT imuDataUtils-export
        FILE
        ImuDataUtilsTargets.cmake
        DESTINATION ${INSTALL_CMAKE_DIR}
        )

################################################################################
# Print configuration
message(STATUS "===============================================================")
message(STATUS "======================  Library Options  ======================")
message(STATUS "Build tests?                              : ${IMUDATAUTILS_BUILD_TESTS}")
message(STATUS "Build command line tool?                  : ${IMUDATAUTILS_BUILD_CMD_LINE_TOOL}")
message(STATUS "===================  Configuration Options  ===================")
message(STATUS "CMAKE_CXX_COMPILER_ID type                : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION                : ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMake version                             : ${CMAKE_VERSION}")
message(STATUS "CMake generator                           : ${CMAKE_GENERATOR}")
message(STATUS "CMake build tool                          : ${CMAKE_BUILD_TOOL}")
message(STATUS "Build flags                                               ")
if(NOT MSVC AND NOT XCODE_VERSION)
    message(STATUS "  Build type                              : ${CMAKE_BUILD_TYPE}")
    message(STATUS "  C compilation flags                     : ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")
    message(STATUS "  C++ compilation flags                   : ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE_UPPER}}")
    if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
        message(STATUS "  C compilation flags (Release)           : ${CMAKE_C_FLAGS_RELEASE}")
        message(STATUS "  C++ compilation flags (Release)         : ${CMAKE_CXX_FLAGS_RELEASE}")
    endif()
    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        message(STATUS "  C compilation flags (Debug)             : ${CMAKE_C_FLAGS_DEBUG}")
        message(STATUS "  C++ compilation flags (Debug)           : ${CMAKE_CXX_FLAGS_DEBUG}")
    endif()
endif()
message(STATUS "===============================================================")

message(STATUS "+++++++++++++++++++   End CMake   ++++++++++++++++++++")

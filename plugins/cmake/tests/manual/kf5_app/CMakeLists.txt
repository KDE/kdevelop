cmake_minimum_required(VERSION 3.0)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${KDevelop_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

find_package(KF5 REQUIRED WidgetsAddons)

add_executable(myprogram main.cpp)
target_link_libraries(myprogram KF${KF_MAJOR_VERSION}::WidgetsAddons)

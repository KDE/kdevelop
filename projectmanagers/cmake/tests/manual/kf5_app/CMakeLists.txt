cmake_minimum_required(VERSION 2.8)

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings)
include(KDECMakeSettings)

find_package(KF5 REQUIRED WidgetsAddons)

add_executable(myprogram main.cpp)
target_link_libraries(myprogram KF5::WidgetsAddons)

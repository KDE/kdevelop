cmake_minimum_required(VERSION 2.8.12)

project(%{APPNAME})

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(KDevPlatform 5.0 REQUIRED)

add_subdirectory(src)

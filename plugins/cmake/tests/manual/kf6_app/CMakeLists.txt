project(kf6_app)

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${KDevelop_SOURCE_DIR}/cmake/modules ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

find_package(KF6 REQUIRED WidgetsAddons)

add_executable(myprogram main.cpp)
target_link_libraries(myprogram KF6::WidgetsAddons)

# also needed for the plugin, so the plugincontrollertest knows the binary dir
#add_definitions( -DBUILD_DIR="\\"${KDevPlatform_BINARY_DIR}\\"" )
include_directories(
    ${KDevPlatform_SOURCE_DIR}
    ${KDevPlatform_SOURCE_DIR}/interfaces
    ${KDevPlatform_SOURCE_DIR}/project
)

ecm_add_test(test_projectmodel.cpp
    LINK_LIBRARIES Qt5::Test KDev::Interfaces KDev::Project KDev::Language KDev::Tests)

set( projectmodelperformancetest_SRCS
    projectmodelperformancetest.cpp
)
add_executable(projectmodelperformancetest ${projectmodelperformancetest_SRCS})
target_link_libraries(projectmodelperformancetest
    Qt5::Test
    Qt5::Gui
    KDev::Interfaces
    KDev::Project
    KDev::Language
    KDev::Tests
)

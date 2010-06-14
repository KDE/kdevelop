# The executables go into the tests dir
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# also needed for the plugin, so the plugincontrollertest knows the binary dir
#add_definitions( -DBUILD_DIR="\\"${KDevPlatform_BINARY_DIR}\\"" )
include_directories(
    ${KDevPlatform_SOURCE_DIR}
    ${KDevPlatform_SOURCE_DIR}/interfaces
    ${KDevPlatform_SOURCE_DIR}/project
    )

set( projectmodeltest_SRCS projectmodeltest.cpp )
kde4_add_unit_test(projectmodeltest ${projectmodeltest_SRCS})
target_link_libraries(projectmodeltest ${QT_QTTEST_LIBRARY} kdevplatforminterfaces kdevplatformproject )


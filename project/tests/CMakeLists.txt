# also needed for the plugin, so the plugincontrollertest knows the binary dir
#add_definitions( -DBUILD_DIR="\\"${KDevPlatform_BINARY_DIR}\\"" )
include_directories(
    ${KDevPlatform_SOURCE_DIR}
    ${KDevPlatform_SOURCE_DIR}/interfaces
    ${KDevPlatform_SOURCE_DIR}/project
)

set( projectmodeltest_SRCS
    projectmodeltest.cpp
)

add_executable(projectmodeltest ${projectmodeltest_SRCS})
add_test(projectmodeltest projectmodeltest)
ecm_mark_as_test(projectmodeltest)
target_link_libraries(projectmodeltest
    Qt5::Test
    kdevplatforminterfaces
    kdevplatformproject
    kdevplatformlanguage
    kdevplatformtests
)

set( projectmodelperformancetest_SRCS
    projectmodelperformancetest.cpp
)
add_executable(projectmodelperformancetest ${projectmodelperformancetest_SRCS})
target_link_libraries(projectmodelperformancetest
    Qt5::Test
    Qt5::Gui
    kdevplatforminterfaces
    kdevplatformproject
    kdevplatformlanguage
    kdevplatformtests
)

###########

set( pathtest_SRCS
    testpath.cpp
)

kde4_add_unit_test(pathtest ${pathtest_SRCS})
target_link_libraries(pathtest
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatformtests
    kdevplatformlanguage
    kdevplatformproject
)

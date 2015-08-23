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

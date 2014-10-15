include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(test_definesandincludes_SRCS
    test_definesandincludes.cpp
    projectsgenerator.cpp)

ecm_add_test(${test_definesandincludes_SRCS}
    TEST_NAME test_definesandincludes
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Project)

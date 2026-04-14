remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

configure_file("paths.h.cmake" "cmake-test-paths.h" ESCAPE_QUOTES)

ecm_add_test(test_cmakebuilder.cpp
    LINK_LIBRARIES
        Qt::Test
        Qt::Core
        KDev::Interfaces
        KDev::Project
        KDev::Tests
        KDevCMakeCommon
)

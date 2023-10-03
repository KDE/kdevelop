ecm_add_test(test_ivariablecontroller.cpp LINK_LIBRARIES
    Qt5::Core
    Qt5::Test
    KDev::Tests
    KDev::Debugger
)

remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)

ecm_add_test(test_breakpointmodel.cpp LINK_LIBRARIES
    Qt5::Core
    Qt5::Test
    KDev::Tests
    KDev::Debugger
)

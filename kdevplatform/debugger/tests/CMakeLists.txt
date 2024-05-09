remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)

foreach(_testFilename test_breakpoint.cpp test_breakpointmodel.cpp test_ivariablecontroller.cpp)
    ecm_add_test(${_testFilename} LINK_LIBRARIES
        Qt::Core
        Qt::Test
        KDev::Tests
        KDev::Debugger
    )
endforeach()

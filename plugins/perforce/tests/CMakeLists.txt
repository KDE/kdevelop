include_directories(
    ..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
set(perforceplugintest_SRCS
    test_perforce.cpp
    ../perforceplugin.cpp
    ../ui/perforceimportmetadatawidget.cpp
    ${kdevperforce_LOG_PART_SRCS}
)

if(WIN32)
    add_definitions( "-DP4_CLIENT_STUB_EXE=\"${CMAKE_CURRENT_BINARY_DIR}/../../../bin/p4clientstub.exe\"" )
else()
    add_definitions( "-DP4_CLIENT_STUB_EXE=\"${CMAKE_CURRENT_BINARY_DIR}/../p4clientstub/p4clientstub\"" )
endif()

ecm_add_test(${perforceplugintest_SRCS}
        TEST_NAME test_kdevperforce
        LINK_LIBRARIES Qt5::Test KDev::Vcs KDev::Util KDev::Tests )


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

add_definitions( "-DP4_BINARY_DIR=\"${PROJECT_BINARY_DIR}\"")

ecm_add_test(${perforceplugintest_SRCS}
        TEST_NAME test_kdevperforce
        LINK_LIBRARIES Qt5::Test KDev::Vcs KDev::Util KDev::Tests )


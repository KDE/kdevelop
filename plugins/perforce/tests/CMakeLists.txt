remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

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
ki18n_wrap_ui(perforceplugintest_SRCS
    ../ui/perforceimportmetadatawidget.ui
)

add_definitions( "-DP4_BINARY_DIR=\"${PROJECT_BINARY_DIR}\"")

ecm_add_test(${perforceplugintest_SRCS}
        TEST_NAME test_kdevperforce
        LINK_LIBRARIES Qt::Test KDev::Vcs KDev::Util KDev::Tests KF6::KIOWidgets)


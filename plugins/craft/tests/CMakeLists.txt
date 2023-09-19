include_directories(
    ..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

set(test_craftruntime_SRCS
    test_craftruntime.cpp
    ../craftruntime.cpp
    ${craftplugin_LOG_SRCS}
)

ecm_add_test(${test_craftruntime_SRCS}
    TEST_NAME test_craftruntime
    LINK_LIBRARIES Qt::Test KDev::Tests)

target_compile_definitions(test_craftruntime PRIVATE -DCRAFT_ROOT_MOCK="${CMAKE_CURRENT_SOURCE_DIR}/craft_root_mock")

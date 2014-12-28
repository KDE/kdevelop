add_executable(qmlcompletiontest qmlcompletiontest.cpp)
add_test(qmlcompletiontest qmlcompletiontest)

target_link_libraries(qmlcompletiontest
    Qt5::Test
    KDev::Tests
    kdevqmljscompletion
)

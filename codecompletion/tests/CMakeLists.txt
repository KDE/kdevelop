add_executable(qmlcompletiontest qmlcompletiontest.cpp)
add_test(qmlcompletiontest qmlcompletiontest)

target_link_libraries(qmlcompletiontest
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevqmljscompletion
)

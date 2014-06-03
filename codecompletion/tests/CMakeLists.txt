kde4_add_unit_test(qmlcompletiontest qmlcompletiontest.cpp)

target_link_libraries(qmlcompletiontest
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevqmljscompletion
)

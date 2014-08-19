add_executable(test_projectfilter
    test_projectfilter.cpp
    ../projectfilter.cpp
    ../filter.cpp
)
add_test(test_projectfilter test_projectfilter)
ecm_mark_as_test(test_projectfilter)

target_link_libraries(test_projectfilter
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${QT_QTTEST_LIBRARIES}
)

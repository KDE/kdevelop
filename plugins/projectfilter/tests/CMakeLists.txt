add_executable(testprojectfilter
    test_projectfilter.cpp
    ../projectfilter.cpp
    ../filter.cpp
)
ecm_mark_as_test(testprojectfilter)

target_link_libraries(testprojectfilter
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${QT_QTTEST_LIBRARIES}
)

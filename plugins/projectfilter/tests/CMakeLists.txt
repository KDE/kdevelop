kde4_add_unit_test(testprojectfilter
    test_projectfilter.cpp
    ../projectfilter.cpp
)

target_link_libraries(testprojectfilter
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${QT_QTTEST_LIBRARIES}
)

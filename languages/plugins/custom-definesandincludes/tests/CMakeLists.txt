set(test_definesandincludes_SRCS
    test_definesandincludes.cpp
    projectsgenerator.cpp)

ecm_add_test(${test_definesandincludes_SRCS}
    TEST_NAME test_definesandincludes
    LINK_LIBRARIES ${QT_QTTEST_LIBRARY} ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES})

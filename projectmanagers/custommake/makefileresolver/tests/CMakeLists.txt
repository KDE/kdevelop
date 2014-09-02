set(test_custommake_SRCS
     test_custommake.cpp
     ../makefileresolver.cpp
)

ecm_add_test(${test_custommake_SRCS}
    TEST_NAME test_custommake
    LINK_LIBRARIES
        ${QT_QTTEST_LIBRARY}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
)

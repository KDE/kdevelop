kde4_add_unit_test(test_compilerprovider
    test_compilerprovider.cpp
)

target_link_libraries(test_compilerprovider
     ${KDEVPLATFORM_TESTS_LIBRARIES}
     ${QT_QTTEST_LIBRARY}
     kdevcompilerprovider
)

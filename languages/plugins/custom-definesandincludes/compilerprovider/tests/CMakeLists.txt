add_executable(test_compilerprovider
    test_compilerprovider.cpp
    ../icompiler.cpp
)
ecm_mark_as_test(test_compilerprovider)
add_test(test_compilerprovider test_compilerprovider)
target_link_libraries(test_compilerprovider
     ${KDEVPLATFORM_TESTS_LIBRARIES}
     ${QT_QTTEST_LIBRARY}
)

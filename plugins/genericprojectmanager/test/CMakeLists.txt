kde4_add_unit_test( reloadtest reloadtest.cpp )
target_link_libraries(reloadtest
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)

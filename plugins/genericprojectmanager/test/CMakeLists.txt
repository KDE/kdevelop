add_executable( reloadtest reloadtest.cpp )
ecm_mark_as_test( reloadtest)
target_link_libraries(reloadtest
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
)

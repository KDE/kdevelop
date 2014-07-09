add_executable(definesandincludestest
    definesandincludestest.cpp
    projectsgenerator.cpp
)
ecm_mark_as_test(definesandincludestest)
add_test(definesandincludestest definesandincludestest)
target_link_libraries( definesandincludestest
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
)

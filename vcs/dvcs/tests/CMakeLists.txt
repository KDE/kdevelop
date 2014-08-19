set(test_dvcs_SRCS test_dvcsjob.cpp)
add_executable(test_dvcs ${test_dvcs_SRCS})
add_test(test_dvcs test_dvcs)
ecm_mark_as_test(test_dvcs)
target_link_libraries(test_dvcs
                      Qt5::Test
                      Qt5::Gui

                      ${KDEVPLATFORM_TESTS_LIBRARIES}
                      KDev::Util
                      KDev::Vcs
)

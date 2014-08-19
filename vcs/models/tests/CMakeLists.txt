set(test_models_SRCS test_models.cpp)
add_executable(test_models ${test_models_SRCS})
add_test(test_models test_models)
ecm_mark_as_test(test_models)
target_link_libraries(test_models
                      Qt5::Test
                      Qt5::Gui

                      ${KDEVPLATFORM_TESTS_LIBRARIES}
                      KDev::Util
                      KDev::Vcs
)

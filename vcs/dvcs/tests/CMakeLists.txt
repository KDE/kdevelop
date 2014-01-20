set(dvcsTest_SRCS dvcsjobTest.cpp)
add_executable(dvcsTest ${dvcsTest_SRCS})
ecm_mark_as_test(dvcsTest)
target_link_libraries(dvcsTest
                      Qt5::Test
                      Qt5::Gui

                      ${KDEVPLATFORM_TESTS_LIBRARIES}
                      kdevplatformutil
                      kdevplatformvcs
)

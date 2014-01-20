set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
set(modelsTest_SRCS modelstest.cpp)
add_executable(modelsTest ${modelsTest_SRCS})
ecm_mark_as_test(modelsTest)
target_link_libraries(modelsTest
                      Qt5::Test
                      Qt5::Gui

                      ${KDEVPLATFORM_TESTS_LIBRARIES}
                      kdevplatformutil
                      kdevplatformvcs
)

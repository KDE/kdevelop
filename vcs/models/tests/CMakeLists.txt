set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
set(modelsTest_SRCS modelstest.cpp)
kde4_add_unit_test(modelsTest ${modelsTest_SRCS})
target_link_libraries(modelsTest
                      ${QT_QTTEST_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      ${KDEVPLATFORM_TESTS_LIBRARIES}
                      kdevplatformutil
                      kdevplatformvcs
                      )

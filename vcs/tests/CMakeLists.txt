set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(vcsBlackboxTest_SRCS vcsBlackBoxTest.cpp)
kde4_add_unit_test(vcsBlackBoxTest ${vcsBlackboxTest_SRCS})
target_link_libraries(vcsBlackBoxTest
                      ${QT_QTTEST_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      kdevplatformutil
                      kdevplatformvcs
                      kdevplatformtests
                     )
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

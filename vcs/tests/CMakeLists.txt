set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

# TODO: renable once someone knows what this is supposed to do and the code is cleaned up
# set(vcsBlackboxTest_SRCS vcsBlackBoxTest.cpp)
# add_executable(vcsBlackBoxTest ${vcsBlackboxTest_SRCS})
# ecm_mark_as_test(vcsBlackBoxTest)
# target_link_libraries(vcsBlackBoxTest
#                       ${QT_QTTEST_LIBRARY}
#                       ${QT_QTGUI_LIBRARY}
#                       ${KDE4_KDECORE_LIBS}
#                       kdevplatformutil
#                       kdevplatformvcs
#                       kdevplatformtests
#                      )
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

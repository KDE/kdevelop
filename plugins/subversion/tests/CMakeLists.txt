set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
set(svnrecursiveadd_SRCS svnrecursiveadd.cpp)
kde4_add_unit_test(svnrecursiveadd ${svnrecursiveadd_SRCS})

target_link_libraries(svnrecursiveadd
                      ${QT_QTTEST_LIBRARY}
                      ${QT_QTGUI_LIBRARY}
                      ${KDE4_KDECORE_LIBS}
                      kdevplatformtests
                      kdevplatformutil
                      kdevplatformvcs
                      )
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )


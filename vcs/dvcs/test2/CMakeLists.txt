set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
set(dvcsTest_SRCS dvcsjobTest.cpp)
kde4_add_executable(dvcsTest2 ${dvcsTest_SRCS})
target_link_libraries(dvcsTest2
		              ${QT_LIBS}
					  ${QT_QTGUI_LIBRARY}
                      ${QT_QTTEST_LIBRARY}      
		              ${KDE4_KDECORE_LIBS}
                      kdevplatformutil
                      kdevplatformvcs
	                  )

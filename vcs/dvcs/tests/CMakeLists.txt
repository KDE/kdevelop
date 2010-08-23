set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
set(dvcsTest_SRCS dvcsjobTest.cpp)
kde4_add_unit_test(dvcsTest ${dvcsTest_SRCS})
target_link_libraries(dvcsTest
		              ${QT_QTTEST_LIBRARY}
					  ${QT_QTGUI_LIBRARY}
		              ${KDE4_KDECORE_LIBS}
                      ${KDEVPLATFORM_TESTS_LIBRARIES}
                      kdevplatformutil
                      kdevplatformvcs
	                  )

if(KDE4_BUILD_TESTS)

include_directories(${CMAKE_SOURCE_DIR}/buildtools/importers/cmake/parser ${CMAKE_SOURCE_DIR}/buildtools/importers/qmake/parser)


set( parsetest_SRCS parsetest.cpp )
kde4_automoc(${parsetest_SRCS})
kde4_add_executable(qmake-parsetest NOGUI RUN_UNINSTALLED ${parsetest_SRCS})
target_link_libraries(qmake-parsetest
	${QT_QTTEST_LIBRARY}
	${KDE4_KDECORE_LIBS}
	kdevplatform
	kdevqmakeparser
	)

add_test(qmake-parsetest ${EXECUTABLE_OUTPUT_PATH}/qmake-parsetest)

endif(KDE4_BUILD_TESTS)

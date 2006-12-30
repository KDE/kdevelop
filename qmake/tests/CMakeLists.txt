if(KDE4_BUILD_TESTS)

include_directories(${CMAKE_SOURCE_DIR}/buildtools/importers/qmake/parser
                    ${CMAKE_SOURCE_DIR}/buildtools/importers/qmake)

set(parsetest_SRCS parsetest.cpp)
kde4_automoc(qmake-parsetest ${parsetest_SRCS})
kde4_add_executable(qmake-parsetest NOGUI RUN_UNINSTALLED ${parsetest_SRCS})
target_link_libraries(qmake-parsetest
	${QT_QTTEST_LIBRARY}
	${KDE4_KDECORE_LIBS}
	kdevplatform
	kdevqmakeparser
	)

set(assignmenttest_SRCS assignmenttest.cpp)
kde4_automoc(qmake-assignmenttest ${assignmenttest_SRCS})
kde4_add_executable(qmake-assignmenttest NOGUI RUN_UNINSTALLED ${assignmenttest_SRCS})
target_link_libraries(qmake-assignmenttest
    ${QT_QTTEST_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    kdevplatform
    kdevqmakeparser
    )

add_test(qmake-parsetest ${EXECUTABLE_OUTPUT_PATH}/qmake-parsetest)
add_test(qmake-assignmenttest ${EXECUTABLE_OUTPUT_PATH}/qmake-assignmenttest)

endif(KDE4_BUILD_TESTS)

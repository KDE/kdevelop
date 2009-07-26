# Due to the use of system() and some unix-style paths this test will only run
# under Linux. (Maybe this can be fixed later)
#
# Moreover, I'm not sure if there is a cvs commandline client for windows
# (need to check this out ...)

if (UNIX)

# Running the test only makes sense if the mercurial command line client
# is present. So check for it before adding the test...
FIND_PROGRAM(MERCURIAL NAMES hg
	PATHS
	/bin
	/usr/bin
	/usr/local/bin
)

if (MERCURIAL)
    set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
	set(mercurialInitTest_SRCS initTest.cpp ../mercurialplugin.cpp)
    kde4_add_unit_test(kdevmercurial-test ${mercurialInitTest_SRCS})
	target_link_libraries(kdevmercurial-test
		${QT_QTTEST_LIBRARY}
		${KDE4_KDECORE_LIBS}
        kdevplatformutil
        kdevplatformvcs
        kdevplatformtests
	)
endif (MERCURIAL)

endif (UNIX)

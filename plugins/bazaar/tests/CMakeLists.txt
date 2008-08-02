# Due to the use of system() and some unix-style paths this test will only run
# under Linux. (Maybe this can be fixed later)
#
# Moreover, I'm not sure if there is a cvs commandline client for windows
# (need to check this out ...)

if (UNIX)

# Running the test only makes sense if the git command line client
# is present. So check for it before adding the test...
FIND_PROGRAM(BZR NAMES bzr
	PATHS
	/bin
	/usr/bin
	/usr/local/bin
)

if (BZR)

    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
	set(bzrInitTest_SRCS initTest.cpp ${CMAKE_SOURCE_DIR}/vcs/dvcs/dvcsjob.cpp ../bzrexecutor.cpp)
    kde4_add_unit_test(kdevbzr-test ${bzrInitTest_SRCS})
	target_link_libraries(kdevbzr-test
		${QT_QTTEST_LIBRARY}
		${KDE4_KDECORE_LIBS}
        kdevplatformutil
        kdevplatformvcs
	)
endif (BZR)

endif (UNIX)

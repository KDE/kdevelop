remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

# Due to the use of system() and some unix-style paths this test will only run
# under Linux. (Maybe this can be fixed later)
#
# Moreover, I'm not sure if there is a cvs commandline client for windows
# (need to check this out ...)

if (UNIX)

# Running the test only makes sense if the cvs command line client
# is present. So check for it before adding the test ...
find_program(CVS NAMES cvs
	PATHS
	/bin
	/usr/bin
	/usr/local/bin
)

if (CVS)
	include_directories(
		..
		${CMAKE_CURRENT_BINARY_DIR}/..
	)
    set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
	set(test_cvs_SRCS test_cvs.cpp
        ../cvsjob.cpp
        ../cvsproxy.cpp
        ../cvsannotatejob.cpp
        ../cvslogjob.cpp
        ../cvsstatusjob.cpp
        ../cvsdiffjob.cpp
        ${kdevcvs_LOG_PART_SRCS}
    )
    ecm_add_test(${test_cvs_SRCS}
        TEST_NAME test_cvs
        LINK_LIBRARIES
            Qt5::Test
            KDev::Util
            KDev::Vcs
            KDev::Tests
	)

endif ()

endif ()


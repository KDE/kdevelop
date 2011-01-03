# Due to the use of system() and some unix-style paths this test will only run
# under Linux. (Maybe this can be fixed later)
#
# Moreover, I'm not sure if there is a cvs commandline client for windows
# (need to check this out ...)

if (UNIX)

# Running the test only makes sense if the git command line client
# is present. So check for it before adding the test...
FIND_PROGRAM(GIT NAMES git
	PATHS
	/bin
	/usr/bin
	/usr/local/bin
)

if (GIT)

    set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )
    set(gitInitTest_SRCS initTest.cpp ../gitplugin.cpp ../gitclonejob.cpp ../stashmanagerdialog.cpp ../gitjob.cpp)
    kde4_add_ui_files(gitInitTest_SRCS ../stashmanagerdialog.ui)
    kde4_add_unit_test(kdevgit-test ${gitInitTest_SRCS})
    target_link_libraries(kdevgit-test
        ${QT_QTTEST_LIBRARY}
        ${KDE4_KDECORE_LIBS}
        ${KDEVPLATFORM_VCS_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
    )
endif (GIT)

endif (UNIX)

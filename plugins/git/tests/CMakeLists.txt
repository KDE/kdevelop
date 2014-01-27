# Due to the use of system() and some unix-style paths this test will only run
# under Linux. (Maybe this can be fixed later)
#

# Running the test only makes sense if the git command line client
# is present. So check for it before adding the test...
find_program(GIT_FOUND NAMES git)

if (GIT_FOUND)

    set(gitInitTest_SRCS
        initTest.cpp
        ../gitplugin.cpp
        ../gitclonejob.cpp
        ../stashmanagerdialog.cpp
        ../stashpatchsource.cpp
        ../gitjob.cpp
        ../gitmessagehighlighter.cpp
        ../gitplugincheckinrepositoryjob.cpp
    )
    qt5_wrap_ui(gitInitTest_SRCS ../stashmanagerdialog.ui)
    add_executable(kdevgit-test ${gitInitTest_SRCS})
    add_test(kdevgit-test kdevgit-test)
ecm_mark_as_test(kdevgit-test)
    target_link_libraries(kdevgit-test
        Qt5::Test
        
        ${KDEVPLATFORM_VCS_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_TESTS_LIBRARIES}
    )
endif ()

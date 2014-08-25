# Running the test only makes sense if the git command line client
# is present. So check for it before adding the test...
find_program(GIT_FOUND NAMES git)

if (GIT_FOUND)

    set(gittest_SRCS
        test_git.cpp
        ../gitplugin.cpp
        ../gitclonejob.cpp
        ../stashmanagerdialog.cpp
        ../stashpatchsource.cpp
        ../gitjob.cpp
        ../gitmessagehighlighter.cpp
        ../gitplugincheckinrepositoryjob.cpp
    )
    qt5_wrap_ui(gittest_SRCS ../stashmanagerdialog.ui)
    ecm_add_test(${gittest_SRCS}
        TEST_NAME test_kdevgit
        LINK_LIBRARIES Qt5::Test ${KDEVPLATFORM_VCS_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES}
        GUI)
endif ()

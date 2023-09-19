remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

configure_file("paths.h.cmake" "cmake-test-paths.h" ESCAPE_QUOTES)

set(commonlibs Qt::Test Qt::Core KDev::Interfaces KDevCMakeCommon kdevcmakemanagernosettings)

ecm_add_test(cmakeparsertest.cpp ../parser/cmListFileLexer.c TEST_NAME test_cmakeparser LINK_LIBRARIES ${commonlibs})
ecm_add_test(test_cmakemanager.cpp    LINK_LIBRARIES ${commonlibs} KDev::Language KDev::Tests KDev::Project)
ecm_add_test(test_ctestfindsuites.cpp LINK_LIBRARIES ${commonlibs} KDev::Language KDev::Tests)
ecm_add_test(test_cmakeserver.cpp     LINK_LIBRARIES ${commonlibs} KDev::Language KDev::Tests KDev::Project)
ecm_add_test(test_cmakefileapi.cpp    LINK_LIBRARIES ${commonlibs} KDev::Language KDev::Tests KDev::Project)

# this is not a unit test but a testing tool, kept here for convenience
add_executable(kdevprojectopen kdevprojectopen.cpp)
target_link_libraries(kdevprojectopen Qt::Test KDev::Project KDev::Tests KDevCMakeCommon)

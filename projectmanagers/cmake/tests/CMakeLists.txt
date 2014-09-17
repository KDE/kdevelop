include_directories(
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake/parser
    ${KDEVPLATFORM_INCLUDE_DIR}
)

configure_file("paths.h.cmake" "cmake-test-paths.h" ESCAPE_QUOTES)

macro(kdevcmake_add_test testname)
    ecm_add_test(${testname}.cpp
        LINK_LIBRARIES Qt5::Test Qt5::Core KDev::Interfaces kdevcmakecommon ${ARGN})
endmacro(kdevcmake_add_test)

# kdevcmake_add_test(cmakeparsertest)
# kdevcmake_add_test(cmakeastfactorytest)
# kdevcmake_add_test(cmakeasttest)
# kdevcmake_add_test(generationexpressionsolvertest)
# kdevcmake_add_test(cmakecompliance KDev::Tests KDev::Language)
# kdevcmake_add_test(cmakeconditiontest KDev::Language KF5::TextEditor)
# kdevcmake_add_test(cmakeduchaintest KDev::Language KF5::TextEditor KDev::Tests)
# kdevcmake_add_test(cmakeprojectvisitortest KDev::Language KF5::TextEditor  KDev::Tests)
# kdevcmake_add_test(cmakeparserutilstest KF5::TextEditor)
# kdevcmake_add_test(cmakeloadprojecttest KDev::Language KDev::Tests)
kdevcmake_add_test(test_cmakemanager KDev::Language KDev::Tests KDev::Project)
# kdevcmake_add_test(ctestfindsuitestest KDev::Language KDev::Tests)

# this is not a unit test but a testing tool, kept here for convenience
add_executable(kdevprojectopen kdevprojectopen.cpp)
target_link_libraries(kdevprojectopen Qt5::Test KDev::Project KDev::Tests kdevcmakecommon)

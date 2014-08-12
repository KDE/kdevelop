include_directories(
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake/parser
    ${KDEVPLATFORM_INCLUDE_DIR}
)

configure_file("paths.h.cmake" "cmake-test-paths.h" ESCAPE_QUOTES)

macro(kdevcmake_add_test testname)
    ecm_add_test(${testname}.cpp
        LINK_LIBRARIES Qt5::Test Qt5::Core ${KDEVPLATFORM_INTERFACES_LIBRARIES} kdevcmakecommon ${ARGN})
endmacro(kdevcmake_add_test)

# kdevcmake_add_test(cmakeparsertest)
# kdevcmake_add_test(cmakeastfactorytest)
# kdevcmake_add_test(cmakeasttest)
# kdevcmake_add_test(generationexpressionsolvertest)
# kdevcmake_add_test(cmakecompliance ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES})
# kdevcmake_add_test(cmakeconditiontest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} KF5::TextEditor)
# kdevcmake_add_test(cmakeduchaintest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} KF5::TextEditor ${KDEVPLATFORM_TESTS_LIBRARIES})
# kdevcmake_add_test(cmakeprojectvisitortest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} KF5::TextEditor  ${KDEVPLATFORM_TESTS_LIBRARIES})
# kdevcmake_add_test(cmakeparserutilstest KF5::TextEditor)
# kdevcmake_add_test(cmakeloadprojecttest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})
# kdevcmake_add_test(cmakemanagertest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES})
# kdevcmake_add_test(ctestfindsuitestest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

# this is not a unit test but a testing tool, kept here for convenience
add_executable(kdevprojectopen kdevprojectopen.cpp)
target_link_libraries(kdevprojectopen Qt5::Test ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} kdevcmakecommon)

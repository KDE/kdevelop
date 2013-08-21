include_directories(
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake
    ${KDevelop_SOURCE_DIR}/projectmanagers/cmake/parser
    ${KDEVPLATFORM_INCLUDE_DIR}
)

configure_file("paths.h.cmake" "cmake-test-paths.h" ESCAPE_QUOTES)

macro(kdevcmake_add_test testname)
    kde4_add_unit_test(${testname} ${testname}.cpp)
    target_link_libraries(${testname} ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} ${KDE4_KDECORE_LIBS} kdev4cmakecommon ${ARGN})
endmacro(kdevcmake_add_test)

kdevcmake_add_test(cmakeparsertest)
kdevcmake_add_test(cmakeastfactorytest)
kdevcmake_add_test(cmakeasttest)
kdevcmake_add_test(generationexpressionsolvertest)
kdevcmake_add_test(cmakecompliance ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES})
kdevcmake_add_test(cmakeconditiontest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDE4_KTEXTEDITOR_LIBS})
kdevcmake_add_test(cmakeduchaintest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDE4_KTEXTEDITOR_LIBS} ${KDEVPLATFORM_TESTS_LIBRARIES})
kdevcmake_add_test(cmakeprojectvisitortest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDE4_KTEXTEDITOR_LIBS}  ${KDEVPLATFORM_TESTS_LIBRARIES})
kdevcmake_add_test(cmakeparserutilstest ${KDE4_KTEXTEDITOR_LIBS})
kdevcmake_add_test(cmakeloadprojecttest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})
kdevcmake_add_test(cmakemanagertest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES})
# kdevcmake_add_test(ctestfindsuitestest ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

# this is not a unit test but a testing tool, kept here for convenience
kde4_add_executable(kdevprojectopen kdevprojectopen.cpp)
target_link_libraries(kdevprojectopen ${KDE4_KDECORE_LIBS} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_TESTS_LIBRARIES})

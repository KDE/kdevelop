set(documentchangesettest_SRCS test_documentchangeset.cpp)

add_executable(documentchangesettest ${documentchangesettest_SRCS})
ecm_mark_as_test(documentchangesettest)
target_link_libraries(documentchangesettest Qt5::Test kdevplatformtests kdevplatformlanguage)


# TODO: uncomment when Grantlee is ported to Qt5
# set(templaterenderertest_SRCS test_templaterenderer.cpp)
#
# add_executable(templaterenderertest ${templaterenderertest_SRCS})
# ecm_mark_as_test(templaterenderertest)
# target_link_libraries(templaterenderertest Qt5::Test kdevplatformtests kdevplatformlanguage)
#
# set(templateclassgeneratortest_SRCS test_templateclassgenerator.cpp)
#
# add_executable(templateclassgeneratortest ${templateclassgeneratortest_SRCS})
# ecm_mark_as_test(templateclassgeneratortest)
# target_link_libraries(templateclassgeneratortest Qt5::Test kdevplatformtests kdevplatformlanguage)
#
# set(templatesmodeltest_SRCS test_templatesmodel.cpp)
#
# add_executable(templatesmodeltest ${templatesmodeltest_SRCS})
# ecm_mark_as_test(templatesmodeltest)
# target_link_libraries(templatesmodeltest Qt5::Test kdevplatformtests kdevplatformlanguage)
#
# configure_file(codegen_tests_config.h.cmake codegen_tests_config.h)
# add_subdirectory(data)

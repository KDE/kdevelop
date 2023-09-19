remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
)

ecm_add_test(test_documentchangeset.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Language)

ecm_add_test(test_templaterenderer.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Language)

ecm_add_test(test_templateclassgenerator.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Language)

ecm_add_test(test_templatesmodel.cpp
    LINK_LIBRARIES Qt::Test KDev::Tests KDev::Language)

configure_file(codegen_tests_config.h.cmake codegen_tests_config.h)
add_subdirectory(data)

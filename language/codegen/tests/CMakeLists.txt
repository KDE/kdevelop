ecm_add_test(test_documentchangeset.cpp
    LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Language)

if (Grantlee_FOUND)
    ecm_add_test(test_templaterenderer.cpp
        LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Language)

    ecm_add_test(test_templateclassgenerator.cpp
        LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Language)

    ecm_add_test(test_templatesmodel.cpp
        LINK_LIBRARIES Qt5::Test KDev::Tests KDev::Language)
endif()

configure_file(codegen_tests_config.h.cmake codegen_tests_config.h)
add_subdirectory(data)

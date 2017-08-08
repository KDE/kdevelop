configure_file(tests_config.h.cmake tests_config.h)

add_subdirectory(data/testgenerationtest/templates)

ecm_add_test(test_generationtest.cpp LINK_LIBRARIES
    Qt5::Test
    KDev::Tests
    KDev::Language
)

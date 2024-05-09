remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

configure_file(tests_config.h.cmake tests_config.h)

add_subdirectory(data/testgenerationtest/templates)

ecm_add_test(test_generationtest.cpp LINK_LIBRARIES
    Qt::Test
    KDev::Tests
    KDev::Language
)

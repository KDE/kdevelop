remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

add_executable(qml-parser
    qml-parser.cpp
)
target_link_libraries(qml-parser
    KDev::Tests
    kdevqmljsduchain
)

configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
ecm_add_test(test_files.cpp
    LINK_LIBRARIES
        Qt5::Test
        Qt5::Core
        KDev::Language
        KDev::Tests
        kdevqmljsduchain
)

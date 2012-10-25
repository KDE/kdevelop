kde4_add_executable(qml-parser
    qml-parser.cpp
)
target_link_libraries(qml-parser
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevqmljsduchain
)
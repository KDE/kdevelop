########### next target ###############

kde4_add_executable(clang-parser
    clang-parser.cpp
)
target_link_libraries(clang-parser
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevclangduchain
)

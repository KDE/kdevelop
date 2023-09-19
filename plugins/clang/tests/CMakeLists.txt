remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
)

add_executable(clang-parser
    clang-parser.cpp
)

target_link_libraries(clang-parser
    KDev::Tests
    KDevClangPrivate
)

add_library(sanitizer_test_init STATIC sanitizer_test_init.cpp)
target_link_libraries(sanitizer_test_init PRIVATE Qt::Core)

add_library(codecompletiontestbase STATIC codecompletiontestbase.cpp)
target_link_libraries(codecompletiontestbase PUBLIC
    KDev::Tests
    Qt::Test
    KDevClangPrivate
)

add_executable(clang-minimal-visitor WIN32
    minimal_visitor.cpp
)
ecm_mark_nongui_executable(clang-minimal-visitor)
target_link_libraries(clang-minimal-visitor
    Clang::libclang
)

ecm_add_test(test_buddies.cpp
    TEST_NAME test_buddies-clang
    LINK_LIBRARIES
        KDev::Tests
        Qt::Test
)

ecm_add_test(test_codecompletion.cpp
    TEST_NAME test_codecompletion
    LINK_LIBRARIES
        codecompletiontestbase
        sanitizer_test_init
)

ecm_add_test(test_assistants.cpp
    TEST_NAME test_assistants
    LINK_LIBRARIES
        KDev::Tests
        Qt::Test
        KDevClangPrivate
        sanitizer_test_init
)

ecm_add_test(test_clangutils.cpp
    TEST_NAME test_clangutils
    LINK_LIBRARIES
        KDev::Tests
        Qt::Test
        Clang::libclang
        KDevClangPrivate
)

ecm_add_test(test_duchain.cpp
    TEST_NAME test_duchain-clang
    LINK_LIBRARIES
        KDev::Tests
        Qt::Test
        KDevClangPrivate
        KDev::DefinesAndIncludesManager
)

ecm_add_test(test_refactoring.cpp
    TEST_NAME test_refactoring-clang
    LINK_LIBRARIES
        KDev::Tests
        Qt::Test
        KDevClangPrivate
)

ecm_add_test(test_duchainutils.cpp
    TEST_NAME test_duchainutils
    LINK_LIBRARIES
        KDev::Tests
        Qt::Test
        KDevClangPrivate
)

ecm_add_test(test_problems.cpp
    TEST_NAME test_problems
    LINK_LIBRARIES
        KDev::Tests
        Qt::Test
        KDevClangPrivate
)

configure_file("testfilepaths.h.cmake" "testfilepaths.h" ESCAPE_QUOTES)
ecm_add_test(test_files.cpp
TEST_NAME test_files-clang
LINK_LIBRARIES
    Qt::Test
    Qt::Core
    KDev::Language
    KDev::Tests
    KDev::DefinesAndIncludesManager
    KDevClangPrivate
)

if(BUILD_BENCHMARKS)
    ecm_add_test(bench_codecompletion.cpp
        TEST_NAME bench_codecompletion
        LINK_LIBRARIES
            codecompletiontestbase
    )
    set_tests_properties(bench_codecompletion PROPERTIES TIMEOUT 30)
    ecm_add_test(bench_duchain.cpp
        TEST_NAME bench_duchain
        LINK_LIBRARIES
            KDev::Tests
            Qt::Test
            KDevClangPrivate
    )
    set_tests_properties(bench_duchain PROPERTIES TIMEOUT 30)
endif()

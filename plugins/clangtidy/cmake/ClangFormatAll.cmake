
file(GLOB_RECURSE ALL_SOURCE_FILES
        src/*/*.cpp
        src/*/*.h
        tests/*.cpp
        tests/*.h
        )

find_program (CLANG_FMT_CMD
        NAMES "clang-format"
        )
if(NOT CLANG_FMT_CMD)
    message(WARNING "clang-format not found!")
else()
    add_custom_target( ClangFormatAll
        COMMAND ${CLANG_FMT_CMD} -style=file -i ${ALL_SOURCE_FILES}
            )
endif()

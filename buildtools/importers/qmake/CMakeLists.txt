add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/project
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
    ${CMAKE_SOURCE_DIR}/lib/interface
    )


########### next target ###############

include(FindFlex)
include(FindBison)

set(kdevqmakeimporter_PART_SRCS
    qmakeimporter.cpp
    qmakemodelitems.cpp
    qmakeprojectscope.cpp
    parser/qmakeast.cpp
    parser/qmakeparser.cpp
)

if(FLEX_EXECUTABLE)
    # Add command to generate the lexer.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/parser/qmake_lexer.ll"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS    -d -o"${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/parser/qmake_lexer.ll"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
#     set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp ${kdevqmakeparser_SRCS})
#     set_source_files_properties(
#         ${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp
#         GENERATED
#         )
else(FLEX_EXECUTABLE)
    message("Assuming existence of the generated lexer file qmake_lexer.cpp")
	add_custom_command(
		OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_lexer.cpp"
		COMMAND ${CP_EXECUTABLE}
		ARGS    "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_lexer.cpp"
		 WORKING_DIRECTORY  "${CMAKE_CURRENT_BINARY_DIR}/"
	)
#     set(kdevqmakeparser_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qmake_lexer.cpp ${kdevqmakeparser_SRCS})
endif(FLEX_EXECUTABLE)

if(BISON_EXECUTABLE)
    # Add command to generate the parser.
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.hpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/parser/qmake_parser.yy"
        DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/qmake_lexer.cpp"
        COMMAND ${BISON_EXECUTABLE}
        ARGS    -r all -k -t -o"${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
                -d "${CMAKE_CURRENT_SOURCE_DIR}/parser/qmake_parser.yy"
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )
#    ADD_CUSTOM_COMMAND(
#        OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.h"
#	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.hpp"
#	COMMAND /bin/mv
#	ARGS "${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.hpp"
#	"${CMAKE_CURRENT_SOURCE_DIR}/qmake_parser.h"
#	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCEDIR}
#	)

    set(kdevqmakeimporter_PART_SRCS ${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp ${kdevqmakeimporter_PART_SRCS})
    set_source_files_properties(
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.hpp"
        GENERATED
        )
else(BISON_EXECUTABLE)
    message("Assuming existence of the generated parser file qmake_parser.cpp")
    set(kdevqmakeimporter_PART_SRCS ${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp ${kdevqmakeimporter_PART_SRCS})
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.cpp"
                "${CMAKE_CURRENT_BINARY_DIR}/qmake_parser.hpp"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_parser.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_parser.hpp"
        COMMAND ${CP_EXECUTABLE}
        ARGS    "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_lexer.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/parser/generated/qmake_lexer.hpp"
                "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif(BISON_EXECUTABLE)


kde4_automoc(${kdevqmakeimporter_PART_SRCS})
kde4_add_plugin(kdevqmakeimporter ${kdevqmakeimporter_PART_SRCS})
target_link_libraries( kdevqmakeimporter ${KDE4_KDECORE_LIBS} kdevplatform kdevqmakeparser )

install(TARGETS kdevqmakeimporter DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES kdevqmakeimporter.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



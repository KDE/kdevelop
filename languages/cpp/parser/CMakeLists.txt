
add_subdirectory(rpp)
add_subdirectory(tests)

include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/editor )

add_definitions(-fPIC)

########### next target ###############

set(kdevcppparser_STAT_SRCS
    ast.cpp
    codemodel_finder.cpp
    default_visitor.cpp
    memorypool.cpp
    type_compiler.cpp
    binder.cpp
    compiler_utils.cpp
    dumptree.cpp
    name_compiler.cpp
    visitor.cpp
    class_compiler.cpp
    control.cpp
    lexer.cpp
    parser.cpp
    codemodel.cpp
    declarator_compiler.cpp
    listnode.cpp
    tokens.cpp
    problem.cpp
    codedisplay.cpp
    dubuilder.cpp
    typesystem.cpp
    duchain.cpp
    ducontext.cpp
    definition.cpp
    identifier.cpp
    cppeditorintegrator.cpp
)

kde4_automoc(${kdevcppparser_STAT_SRCS})
kde4_add_library(kdevcppparser STATIC ${kdevcppparser_STAT_SRCS})


########### install files ###############


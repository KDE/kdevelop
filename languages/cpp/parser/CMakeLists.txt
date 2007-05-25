
add_subdirectory(rpp)
add_subdirectory(tests)
add_subdirectory(headergen)

include_directories(
    ${CMAKE_SOURCE_DIR}/lib/language
    ${CMAKE_SOURCE_DIR}/lib/language/duchain
)


########### next target ###############

set(kdevcppparser_LIB_SRCS
    ast.cpp
#     codemodel_finder.cpp
    default_visitor.cpp
    memorypool.cpp
#     binder.cpp
#     compiler_utils.cpp
    name_compiler.cpp
    visitor.cpp
    class_compiler.cpp
    control.cpp
    lexer.cpp
    parser.cpp
#     codemodel.cpp
#     declarator_compiler.cpp
    listnode.cpp
    tokens.cpp
    problem.cpp
#     codedisplay.cpp
    dumptree.cpp
    parsesession.cpp
    type_compiler.cpp
)

kde4_automoc(${kdevcppparser_LIB_SRCS})
kde4_add_library(kdevcppparser SHARED ${kdevcppparser_LIB_SRCS})
target_link_libraries(kdevcppparser kdevduchain ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} )
set_target_properties(kdevcppparser PROPERTIES VERSION 4.0.0 SOVERSION 4)
install(TARGETS kdevcppparser DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install(FILES
    ast.h
    codemodel_finder.h
    control.h
    name_compiler.h
    rxx_allocator.h
    visitor.h
#     binder.h
#     codemodel_fwd.h
#     declarator_compiler.h
    lexer.h
    parser.h
    symbol.h
    class_compiler.h
#     codemodel.h
    default_visitor.h
    listnode.h
    parsesession.h
    tokens.h
#     codedisplay.h
#     compiler_utils.h
    dumptree.h
    memorypool.h
    problem.h
    type_compiler.h
    cppparserexport.h
    DESTINATION ${INCLUDE_INSTALL_DIR}/kdevelop/cpp/parser
)


add_subdirectory(rpp)
add_subdirectory(tests)
add_subdirectory(headergen)

########### next target ###############

set(kdevcppparser_LIB_SRCS
    ast.cpp
    default_visitor.cpp
    memorypool.cpp
    name_compiler.cpp
    visitor.cpp
    class_compiler.cpp
    control.cpp
    lexer.cpp
    parser.cpp
    listnode.cpp
    tokens.cpp
    dumptree.cpp
    parsesession.cpp
    type_compiler.cpp
    commentparser.cpp
    commentformatter.cpp
)

# Note: This library doesn't follow API/ABI/BC rules and shouldn't have a SOVERSION
#       Its only purpose is to support the plugin without needing to add all source files
#       to the plugin target
kde4_add_library(kdev4cppparser SHARED ${kdevcppparser_LIB_SRCS})
target_link_libraries(kdev4cppparser ${KDEVPLATFORM_LANGUAGE_LIBRARY} ${KDE4_KDECORE_LIBS} ${KDE4_KTEXTEDITOR_LIBS} kdev4cpprpp )
install(TARGETS kdev4cppparser DESTINATION ${LIB_INSTALL_DIR} )

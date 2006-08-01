
include_directories( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces )

add_definitions(-fPIC)

########### next target ###############

set(kdevcsharpparser_STAT_SRCS
    decoder.cpp
    csharp.cpp
    # cmake doesn't know how to deal with .ll files, so take the .cc one
    csharp_lexer.cc
    csharp_io.cpp
    csharp_pp.cpp
    csharp_pp_scope.cpp
    csharp_pp_handler_visitor.cpp
)

kde4_automoc(${kdevcsharpparser_STAT_SRCS})
kde4_add_library(kdevcsharpparser STATIC ${kdevcsharpparser_STAT_SRCS})

SET(kdevcppparser_SRCS  ast.cpp driver.cpp errors.cpp lexer.cpp lookup.cpp parser.cpp tree_parser.cpp)

ADD_LIBRARY(kdevcppparser SHARED ${kdevcppparser_SRCS})
SET(kdevcppparser_LIB_SRCS
ast.cpp
driver.cpp
errors.cpp
lexer.cpp
lookup.cpp
parser.cpp
tree_parser.cpp
)

ADD_LIBRARY(kdevcppparser SHARED ${kdevcppparser_LIB_SRCS})



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#AM_CXXFLAGS = -DKDEVELOP_BGPARSER
#lib_LTLIBRARIES = libkdevcppparser.la
#libkdevcppparser_la_LDFLAGS = $(all_libraries)
#libkdevcppparser_la_SOURCES = ast.cpp driver.cpp errors.cpp lexer.cpp lookup.cpp parser.cpp tree_parser.cpp
#
#kdevcppparserincludedir = $(includedir)/kdevelop/cppparser
#kdevcppparserinclude_HEADERS = ast.h  driver.h lexer.h parser.h tree_parser.h errors.h keywords.lut.h lookup.h
#

#######################
#previous CMakeLists.txt contents follow:

#SET(kdevcppparser_SRCS  ast.cpp driver.cpp errors.cpp lexer.cpp lookup.cpp parser.cpp tree_parser.cpp)
#
#ADD_LIBRARY(kdevcppparser SHARED ${kdevcppparser_SRCS})
INCLUDES = $(all_includes)
AM_CXXFLAGS = -DKDEVELOP_BGPARSER
lib_LTLIBRARIES = libkdevcppparser.la
libkdevcppparser_la_LDFLAGS = $(all_libraries)
libkdevcppparser_la_SOURCES = \
	ast.cpp      errors.cpp  list.cpp  parser.cpp       treedump.cpp \
	control.cpp  lexer.cpp   main.cpp  smallobject.cpp  treewalker.cpp

kdevcppparserincludedir = $(includedir)/kdevelop/cppparser
kdevcppparserinclude_HEADERS = \
	ast.h      errors.h  list.h    rxx_allocator.h  symbol.h  treedump.h \
	control.h  lexer.h   parser.h  smallobject.h    tokens.h  treewalker.h


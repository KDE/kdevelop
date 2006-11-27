INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
AM_CXXFLAGS = -DKDEVELOP_BGPARSER
lib_LTLIBRARIES = libkdevcppparser.la
libkdevcppparser_la_LDFLAGS = $(all_libraries)
libkdevcppparser_la_SOURCES = ast.cpp cachemanager.cpp driver.cpp errors.cpp \
	lexer.cpp lexercache.cpp lookup.cpp parser.cpp tree_parser.cpp

kdevcppparserincludedir = $(includedir)/kdevelop/cppparser
kdevcppparserinclude_HEADERS = ast.h  driver.h lexer.h parser.h tree_parser.h errors.h lookup.h cachemanager.h lexercache.h macro.h


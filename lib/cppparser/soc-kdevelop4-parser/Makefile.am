INCLUDES = $(all_includes)
AM_CXXFLAGS = -DKDEVELOP_BGPARSER

bin_PROGRAMS = rpp0 range_test

rpp0_LDFLAGS = $(all_libraries)
rpp0_SOURCES = ast.cpp control.cpp errors.cpp lexer.cpp list.cpp main.cpp parser.cpp smallobject.cpp treedump.cpp treewalker.cpp

kdevcppparserincludedir = $(includedir)/kdevelop/cppparser
kdevcppparserinclude_HEADERS = ast.h control.h errors.h lexer.h list.h parser.h rxx_allocator.h smallobject.h symbol.h tokens.h treedump.h treewalker.h

range_test_LDFLAGS = $(all_libraries)
range_test_SOURCES = rangenumber_testharn.cpp rangenumber.cpp


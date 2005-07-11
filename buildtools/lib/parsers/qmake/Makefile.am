# This directory collects some classes related to
# project management for the sole purpose that they
# can be shared between parts.

#SUBDIRS = tests

INCLUDES = -I$(top_srcdir)/lib/interfaces \
        -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets/propeditor \
        $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES = libkdevqmakeparser.la
libkdevqmakeparser_la_LDFLAGS = -no-undefined $(all_libraries) $(LIB_KIO)
libkdevqmakeparser_la_SOURCES = qmakeast.cpp qmakedriver.cpp qmake_yacc.cpp \
	qmakeastvisitor.cpp

kdevelopbuildtoolsincludedir = $(includedir)/kdevelop/buildtools/parsers/qmake
kdevelopbuildtoolsinclude_HEADERS = qmakeast.h qmakedriver.h qmakeastvisitor.h

parser:
	cd $(srcdir) ; \
	bison -d qmake.yy -o qmake_yacc.cpp ; \
	mv -f qmake_yacc.hpp qmake_yacc.h ; \
	flex -oqmake_lex.cpp qmake.ll

EXTRA_DIST = qmake.yy qmake.ll

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
DOXYGEN_PROJECTNAME = KDevelop QMake parser
DOXYGEN_DOCDIRPREFIX = kdevparser
include ../../../../Doxyfile.am


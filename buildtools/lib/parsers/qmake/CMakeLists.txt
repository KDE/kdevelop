INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets/propeditor ${CMAKE_CURRENT_SOURCE_DIR})

SET(kdevqmakeparser_LIB_SRCS
qmakeast.cpp
qmakedriver.cpp
qmake_yacc.cpp
)

KDE_AUTOMOC(${kdevqmakeparser_LIB_SRCS})

ADD_LIBRARY(kdevqmakeparser SHARED ${kdevqmakeparser_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevqmakeparser ${QT_LIBRARIES} kdecore kio)

#original Makefile.am contents follow:

## This directory collects some classes related to
## project management for the sole purpose that they
## can be shared between parts.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#        -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets/propeditor \
#        $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES = libkdevqmakeparser.la
#libkdevqmakeparser_la_LDFLAGS = $(all_libraries) $(LIB_KIO)
#libkdevqmakeparser_la_SOURCES = qmakeast.cpp qmakedriver.cpp qmake_yacc.cpp
#
#kdevelopbuildtoolsincludedir = $(includedir)/kdevelop/buildtools/parsers/qmake
#kdevelopbuildtoolsinclude_HEADERS = qmakeast.h qmakedriver.h
#
#parser:
#	cd $(srcdir) ; \
#	bison -d qmake.yy -o qmake_yacc.cpp ; \
#	mv -f qmake_yacc.hpp qmake_yacc.h ; \
#	flex -oqmake_lex.cpp qmake.ll
#
#EXTRA_DIST = qmake.yy qmake.ll
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
#DOXYGEN_PROJECTNAME = KDevelop QMake parser
#DOXYGEN_DOCDIRPREFIX = kdevparser
#include ../../../../Doxyfile.am

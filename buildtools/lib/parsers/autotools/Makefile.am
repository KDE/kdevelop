# This directory collects some classes related to
# project management for the sole purpose that they
# can be shared between parts.
SUBDIRS = . tests
INCLUDES = -I$(top_srcdir)/lib/interfaces \
        -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets/propeditor \
        $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES = libkdevautotoolsparser.la
libkdevautotoolsparser_la_LDFLAGS = $(all_libraries) $(LIB_KIO)
libkdevautotoolsparser_la_SOURCES = autotoolsast.cpp autotoolsdriver.cpp autotools_yacc.cpp

parser:
	cd $(srcdir) ; \
	bison -d autotools.yy -o autotools_yacc.cpp ; \
	mv -f autotools_yacc.hpp autotools_yacc.h ; \
	flex -oautotools_lex.cpp autotools.ll

EXTRA_DIST = autotools.yy autotools.ll

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
DOXYGEN_PROJECTNAME = KDevelop AutoTools Parser
DOXYGEN_DOCDIRPREFIX = kdevparser
include ../../../../Doxyfile.am

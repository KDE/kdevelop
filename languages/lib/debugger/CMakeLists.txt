SET(lang_debugger_LIB_SRCS
kdevdebugger.cpp
debugger.cpp
)

KDE_AUTOMOC(${lang_debugger_LIB_SRCS})

ADD_LIBRARY(lang_debugger SHARED ${lang_debugger_LIB_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES = liblang_debugger.la
#liblang_debugger_la_LDFLAGS = $(all_libraries)
#liblang_debugger_la_SOURCES = kdevdebugger.cpp debugger.cpp
#langincludedirdir = $(includedir)/kdevelop/languages/debugger
#langincludedir_HEADERS = debugger.h kdevdebugger.h
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevinterfaces kdevutil
#DOXYGEN_PROJECTNAME = KDevelop Debugger Support Library
#DOXYGEN_DOCDIRPREFIX = kdevlang
#include ../../../Doxyfile.am

#######################
#previous CMakeLists.txt contents follow:


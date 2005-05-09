INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets/propeditor )

SET(kdevbuildbase_LIB_SRCS
kdevbuildtool.cpp
)

KDE_AUTOMOC(${kdevbuildbase_LIB_SRCS})

ADD_LIBRARY(kdevbuildbase SHARED ${kdevbuildbase_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevbuildbase ${QT_LIBRARIES} kdecore kdeui kdevelop)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets/propeditor \
#	$(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES = libkdevbuildbase.la
#libkdevbuildbase_la_LDFLAGS = $(all_libraries)
#libkdevbuildbase_la_SOURCES = kdevbuildtool.cpp
#kdevelopbuildtoolsincludedir = $(includedir)/kdevelop/buildtools/base
#kdevelopbuildtoolsinclude_HEADERS = kdevbuildtool.h
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevutil kdevinterfaces kdevextensions
#DOXYGEN_PROJECTNAME = KDevelop Buildtool Base Library
#DOXYGEN_DOCDIRPREFIX = kdevbt
#include ../../../Doxyfile.am

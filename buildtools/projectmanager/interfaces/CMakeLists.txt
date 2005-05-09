INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/project ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_CURRENT_SOURCE_DIR})

SET(projectmanager_interfaces_LIB_SRCS
kdevprojectimporter.cpp
kdevprojectmodel.cpp
kdevprojectbuilder.cpp
kdevprojecteditor.cpp
)

KDE_AUTOMOC(${projectmanager_interfaces_LIB_SRCS})

ADD_LIBRARY(projectmanager_interfaces SHARED ${projectmanager_interfaces_LIB_SRCS})

TARGET_LINK_LIBRARIES(projectmanager_interfaces ${QT_LIBRARIES})

#original Makefile.am contents follow:

#INCLUDES = \
#    -I$(top_srcdir)/lib/interfaces \
#    -I$(top_srcdir)/lib/project \
#    -I$(top_srcdir)/lib/util \
#    $(all_includes)
#
#METASOURCES = AUTO
#
#lib_LTLIBRARIES = \
#    libprojectmanager_interfaces.la
#
#libprojectmanager_interfaces_la_SOURCES = kdevprojectimporter.cpp \
#	kdevprojectmodel.cpp kdevprojectbuilder.cpp kdevprojecteditor.cpp
#
#libprojectmanager_interfaces_la_LDFLAGS = \
#    -no-undefined $(all_libraries) -version-info 1:0
#
#libprojectmanager_interfaces_la_LIBADD = \
#    $(top_builddir)/lib/libkdevelop.la
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = \
#    kdevelopprojectimporter.desktop \
#    kdevelopprojectbuilder.desktop
#
#kdeveloppmincludedir = $(includedir)/kdevelop/buildtools/projectmanager/interfaces
#kdeveloppminclude_HEADERS = kdevprojectbuilder.h  kdevprojecteditor.h  kdevprojectimporter.h  kdevprojectmodel.h
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevutil kdevinterfaces
#DOXYGEN_PROJECTNAME = KDevelop Project Manager Interfaces Library
#DOXYGEN_DOCDIRPREFIX = kdevpm
#include ../../../Doxyfile.am

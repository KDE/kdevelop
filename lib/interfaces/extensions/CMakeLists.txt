SUBDIRS( dcop )

SET(kdevextensions_STAT_SRCS
kdevappfrontend.cpp
kdevmakefrontend.cpp
)

KDE_AUTOMOC(${kdevextensions_STAT_SRCS})

ADD_LIBRARY(kdevextensions STATIC ${kdevextensions_STAT_SRCS})



#original Makefile.am contents follow:

#kdevelopincludeextdir = $(includedir)/kdevelop/interfaces/extensions
#kdevelopincludeext_HEADERS = kdevsourceformatter.h kdevappfrontend.h \
#	kdevmakefrontend.h kdevdifffrontend.h kdevcreatefile.h kdevversioncontrol.h
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kdevelopappfrontend.desktop kdevelopcreatefile.desktop \
#	kdevelopdifffrontend.desktop kdevelopmakefrontend.desktop kdevelopsourceformatter.desktop \
#	kdevelopversioncontrol.desktop
#
#SUBDIRS = dcop
#libkdevextensions_la_LDFLAGS = $(all_libraries)
#noinst_LTLIBRARIES = libkdevextensions.la
#libkdevextensions_la_SOURCES = kdevappfrontend.cpp kdevmakefrontend.cpp
#INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
#METASOURCES = AUTO
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevinterfaces
#DOXYGEN_PROJECTNAME = KDevelop Extension Interfaces Library
#DOXYGEN_DOCDIRPREFIX = kdev
#include ../../../Doxyfile.am

#######################
#previous CMakeLists.txt contents follow:

#SUBDIRS(dcop)
#
#SET(kdevextensions_SRCS kdevappfrontend.cpp kdevmakefrontend.cpp)
#
#KDE_AUTOMOC(${kdevextensions_SRCS})
#
#ADD_LIBRARY(kdevextensions STATIC ${kdevextensions_SRCS})
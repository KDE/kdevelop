kdevelopincludeextdir = $(includedir)/kdevelop/interfaces/extensions
kdevelopincludeext_HEADERS = codebrowserfrontend.h kdevappfrontend.h \
	kdevcreatefile.h kdevdifffrontend.h kdevmakefrontend.h kdevquickopen.h \
	kdevsourceformatter.h kdevversioncontrol.h

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kdevelopappfrontend.desktop \
	kdevelopcodebrowserfrontend.desktop kdevelopcreatefile.desktop kdevelopdifffrontend.desktop \
	kdevelopmakefrontend.desktop kdevelopquickopen.desktop kdevelopsourceformatter.desktop \
	kdevelopversioncontrol.desktop

SUBDIRS = dcop
libkdevextensions_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libkdevextensions.la
libkdevextensions_la_SOURCES = kdevappfrontend.cpp kdevmakefrontend.cpp
INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
METASOURCES = AUTO

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevinterfaces
DOXYGEN_PROJECTNAME = KDevelop Extension Interfaces Library
DOXYGEN_DOCDIRPREFIX = kdev
include ../../../Doxyfile.am
noinst_HEADERS = codebrowserfrontend.h

SET(lang_interfaces_LIB_SRCS
kdevpcsimporter.cpp
)

KDE_AUTOMOC(${lang_interfaces_LIB_SRCS})

ADD_LIBRARY(lang_interfaces SHARED ${lang_interfaces_LIB_SRCS})

TARGET_LINK_LIBRARIES(lang_interfaces ${QT_LIBRARIES})

INSTALL_TARGETS(/lib lang_interfaces)

#original Makefile.am contents follow:

#
#METASOURCES = AUTO
#langincludedirdir = $(includedir)/kdevelop/languages/interfaces
#lib_LTLIBRARIES = liblang_interfaces.la
#liblang_interfaces_la_LDFLAGS = $(all_libraries)
#liblang_interfaces_la_SOURCES = kdevpcsimporter.cpp
#liblang_interfaces_la_LIBADD = $(LIB_QT)
#langincludedir_HEADERS = kdevpcsimporter.h
#INCLUDES = $(all_includes)
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kdeveloppcsimporter.desktop
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevinterfaces kdevutil
#DOXYGEN_PROJECTNAME = KDevelop Language Support Interfaces Library
#DOXYGEN_DOCDIRPREFIX = kdevlang
#include ../../../Doxyfile.am

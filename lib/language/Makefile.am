
INCLUDES = $(all_includes)

langincludedir = $(includedir)/kdevelop/language
langinclude_HEADERS = kdevpcsimporter.h

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kdeveloppcsimporter.desktop

lib_LTLIBRARIES = liblang_interfaces.la
liblang_interfaces_la_LDFLAGS = $(all_libraries)
liblang_interfaces_la_SOURCES = kdevpcsimporter.cpp
liblang_interfaces_la_LIBADD = $(LIB_QT) $(LIB_KDEUI) $(LIB_KPARTS)

METASOURCES = AUTO

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevinterfaces kdevutil
DOXYGEN_PROJECTNAME = KDevelop Language Support Interfaces Library
DOXYGEN_DOCDIRPREFIX = kdevlang
include ../../Doxyfile.am


INCLUDES = -I$(top_srcdir)/lib/util $(all_includes)

langincludedir = $(includedir)/kdevelop/language
langinclude_HEADERS = kdevpcsimporter.h kdevcodemodel.h

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kdeveloppcsimporter.desktop

lib_LTLIBRARIES = liblang_interfaces.la
liblang_interfaces_la_LDFLAGS = $(all_libraries)
liblang_interfaces_la_SOURCES = kdevpcsimporter.cpp kdevcodemodel.cpp
liblang_interfaces_la_LIBADD = \
  $(top_builddir)/lib/util/libkdevutil.la \
  $(LIB_QT) $(LIB_KDEUI) $(LIB_KPARTS)

METASOURCES = AUTO

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevinterfaces kdevutil
DOXYGEN_PROJECTNAME = KDevelop Language Support Interfaces Library
DOXYGEN_DOCDIRPREFIX = kdevlang
include ../../Doxyfile.am

INCLUDES = $(all_includes)
lib_LTLIBRARIES = libkdevcatalog.la
libkdevcatalog_la_SOURCES = tag.cpp

kdevcatalogincludedir = $(includedir)/kdevelop/catalog
kdevcataloginclude_HEADERS = gcatalog.h gcatalog.tcc catalog.h tag.h

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
DOXYGEN_PROJECTNAME = KDevelop Catalog Library
DOXYGEN_DOCDIRPREFIX = kdev
include ../../Doxyfile.am

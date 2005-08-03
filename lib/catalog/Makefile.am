INCLUDES = $(DB3INCLUDES) $(all_includes)
lib_LTLIBRARIES = libkdevcatalog.la
libkdevcatalog_la_SOURCES = tag.cpp catalog.cpp
libkdevcatalog_la_LDFLAGS = -no-undefined $(all_libraries)
libkdevcatalog_la_LIBADD = -ldb $(LIB_KDECORE) $(LIB_QT)

kdevcatalogincludedir = $(includedir)/kdevelop/catalog
kdevcataloginclude_HEADERS = catalog.h tag.h

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
DOXYGEN_PROJECTNAME = KDevelop Catalog Library
DOXYGEN_DOCDIRPREFIX = kdev
include ../../Doxyfile.am

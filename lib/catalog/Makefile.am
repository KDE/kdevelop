INCLUDES = $(all_includes) $(DB3INCLUDES)
lib_LTLIBRARIES = libkdevcatalog.la
libkdevcatalog_la_SOURCES = tag.cpp catalog.cpp
libkdevcatalog_la_LDFLAGS = -no-undefined $(all_libraries) $(DB3LDFLAGS)
libkdevcatalog_la_LIBADD = -l$(DB3LIB) $(LIB_KDECORE) $(LIB_QT)

kdevcatalogincludedir = $(includedir)/kdevelop/catalog
kdevcataloginclude_HEADERS = catalog.h tag.h

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
DOXYGEN_PROJECTNAME = KDevelop Catalog Library
DOXYGEN_DOCDIRPREFIX = kdev
include ../../Doxyfile.am

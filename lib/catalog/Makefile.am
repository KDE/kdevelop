INCLUDES = -I$(top_builddir)/bdb/build_unix -I$(top_srcdir)/bdb/include $(all_includes)
lib_LTLIBRARIES = libkdevcatalog.la
libkdevcatalog_la_SOURCES = tag.cpp catalog.cpp
libkdevcatalog_la_LDFLAGS = -no-undefined $(all_libraries) -L$(top_srcdir)/bdb/dist
libkdevcatalog_la_LIBADD = $(top_builddir)/bdb/build_unix/libdb.a \
	$(LIB_KDECORE) $(LIB_QT)

kdevcatalogincludedir = $(includedir)/kdevelop/catalog
kdevcataloginclude_HEADERS = catalog.h tag.h

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
DOXYGEN_PROJECTNAME = KDevelop Catalog Library
DOXYGEN_DOCDIRPREFIX = kdev
include ../../Doxyfile.am

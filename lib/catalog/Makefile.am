INCLUDES = $(all_includes) $(DB3INCLUDES)
lib_LTLIBRARIES = libkdevcatalog.la
libkdevcatalog_la_SOURCES = tag.cpp
libkdevcatalog_la_LDFLAGS = $(DB3LDFLAGS)
libkdevcatalog_la_LIBADD = -l$(DB3LIB)

kdevcatalogincludedir = $(includedir)/kdevelop/catalog
kdevcataloginclude_HEADERS = gcatalog.h gcatalog.tcc catalog.h tag.h

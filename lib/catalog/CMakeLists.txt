SET(kdevcatalog_LIB_SRCS
tag.cpp
)

ADD_LIBRARY(kdevcatalog SHARED ${kdevcatalog_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevcatalog ${QT_LIBRARIES})


#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#lib_LTLIBRARIES = libkdevcatalog.la
#libkdevcatalog_la_SOURCES = tag.cpp
#libkdevcatalog_la_LDFLAGS = $(all_libraries) -no-undefined
#libkdevcatalog_la_LIBADD = $(LIB_QT)
#
#kdevcatalogincludedir = $(includedir)/kdevelop/catalog
#kdevcataloginclude_HEADERS = gcatalog.h gcatalog.tcc catalog.h tag.h
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
#DOXYGEN_PROJECTNAME = KDevelop Catalog Library
#DOXYGEN_DOCDIRPREFIX = kdev
#include ../../Doxyfile.am

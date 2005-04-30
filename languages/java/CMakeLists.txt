SUBDIRS( file_templates app_templates debugger doc )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/antlr ${CMAKE_SOURCE_DIR}/lib/catalog ${CMAKE_SOURCE_DIR}/lib/compat ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/bdb/include/ ${CMAKE_SOURCE_DIR} )

ADD_DEFINITIONS(-fexceptions)

SET(kdevjavasupport_PART_SRCS
JavaLexer.cpp
KDevJavaSupportIface.cpp
javasupportfactory.cpp
JavaRecognizer.cpp
backgroundparser.cpp
javasupportpart.cpp
JavaStoreWalker.cpp
javasupport_utils.cpp
problemreporter.cpp
driver.cpp
kdevdriver.cpp
)

SET( kdevjavasupport_UI
configproblemreporter.ui
)

SET( kdevjavasupport_DCOP_SRCS
KDevJavaSupportIface.h
)

KDE_AUTOMOC(${kdevjavasupport_PART_SRCS})

KDE_ADD_UI_FILES(kdevjavasupport_PART_SRCS ${kdevjavasupport_UI} )

KDE_ADD_DCOP_SKELS(kdevjavasupport_PART_SRCS ${kdevjavasupport_DCOP_SRCS})

ADD_LIBRARY(kdevjavasupport SHARED ${kdevjavasupport_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevjavasupport ${KDEV_LANG_LIBS} DCOP antlr kdevcatalog)

#original Makefile.am contents follow:

## Here resides the Java support part.
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
## SUBDIRS = templates subclassing_template newclass_templates file_templates
#SUBDIRS = file_templates app_templates debugger doc
#
#INCLUDES = -I$(top_srcdir)/lib/antlr -I$(top_srcdir)/lib/catalog \
#	-I$(top_srcdir)/lib/compat -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	-I$(top_builddir)/bdb/build_unix -I$(top_srcdir)/bdb/include $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevjavasupport.la
#libkdevjavasupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevjavasupport_la_LIBADD = $(top_builddir)/bdb/build_unix/libdb.a $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/antlr/src/libantlr.la $(top_builddir)/lib/catalog/libkdevcatalog.la
#
#libkdevjavasupport_la_SOURCES = JavaLexer.cpp KDevJavaSupportIface.cpp javasupportfactory.cpp \
#JavaRecognizer.cpp backgroundparser.cpp configproblemreporter.ui  javasupportpart.cpp \
#JavaStoreWalker.cpp javasupport_utils.cpp problemreporter.cpp driver.cpp kdevdriver.cpp KDevJavaSupportIface.skel
#
#
#EXTRA_DIST = java.g java.tree.g java.store.g
#
#
##JavaLexer.hpp JavaLexer.cpp JavaRecognizer.hpp JavaRecognizer.cpp: # java.g
##       antlr java.g
#
##JavaStoreWalker.hpp JavaStoreWalker.cpp: # java.store.g
##       antlr java.store.g
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevjavasupport.desktop
#
#rcdir = $(kde_datadir)/kdevjavasupport
#rc_DATA = kdevjavasupport.rc

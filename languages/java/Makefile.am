# Here resides the Java support part.

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

# SUBDIRS = templates subclassing_template newclass_templates file_templates
SUBDIRS = file_templates app_templates debugger doc

INCLUDES = -I$(top_srcdir)/lib/antlr -I$(top_srcdir)/lib/catalog \
	-I$(top_srcdir)/lib/compat -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	-I$(top_builddir)/bdb/build_unix -I$(top_srcdir)/bdb/include $(all_includes)

kde_module_LTLIBRARIES = libkdevjavasupport.la
libkdevjavasupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevjavasupport_la_LIBADD = $(top_builddir)/bdb/libdb.la $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/antlr/src/libantlr.la $(top_builddir)/lib/catalog/libkdevcatalog.la

libkdevjavasupport_la_SOURCES = JavaLexer.cpp KDevJavaSupportIface.cpp javasupportfactory.cpp \
JavaRecognizer.cpp backgroundparser.cpp configproblemreporter.ui  javasupportpart.cpp \
JavaStoreWalker.cpp javasupport_utils.cpp problemreporter.cpp driver.cpp kdevdriver.cpp KDevJavaSupportIface.skel


EXTRA_DIST = java.g java.tree.g java.store.g


#JavaLexer.hpp JavaLexer.cpp JavaRecognizer.hpp JavaRecognizer.cpp: # java.g
#       antlr java.g

#JavaStoreWalker.hpp JavaStoreWalker.cpp: # java.store.g
#       antlr java.store.g

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevjavasupport.desktop

rcdir = $(kde_datadir)/kdevjavasupport
rc_DATA = kdevjavasupport.rc

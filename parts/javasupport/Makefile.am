# Here resides the Java support part.

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

# SUBDIRS = templates subclassing_template newclass_templates


INCLUDES = -I$(top_srcdir)/lib/antlr                 -I$(top_srcdir)/lib/interfaces                 -I$(top_srcdir)/lib/sourceinfo                 -I$(top_srcdir)/lib/util                 -I$(top_srcdir)/lib/catalog 		-I$(top_srcdir)/lib/compat                 $(all_includes)

kde_module_LTLIBRARIES = libkdevjavasupport.la
libkdevjavasupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN)
libkdevjavasupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la                                 $(top_builddir)/lib/antlr/src/libantlr.la                                 $(top_builddir)/lib/catalog/libkdevcatalog.la

libkdevjavasupport_la_SOURCES = javasupportpart.cpp ccconfigwidget.cpp ccconfigwidgetbase.ui javanewclassdlgbase.ui javanewclassdlg.cpp problemreporter.cpp configproblemreporter.ui backgroundparser.cpp implementmethodsdialog.ui subclassingdlg.cpp subclassingdlgbase.ui addmethoddialogbase.ui addmethoddialog.cpp addattributedialogbase.ui addattributedialog.cpp KDevJavaSupportIface.cpp KDevJavaSupportIface.skel classgeneratorconfigbase.ui classgeneratorconfig.cpp javasupportfactory.cpp codeinformationrepository.cpp doxydoc.cpp JavaLexer.cpp JavaRecognizer.cpp JavaStoreWalker.cpp driver.cpp

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

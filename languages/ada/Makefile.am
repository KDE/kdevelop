# Here resides the Ada support part.
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

INCLUDES = -I$(top_srcdir)/lib/antlr -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/external_interfaces -I$(top_srcdir)/lib/util $(all_includes)
SUBDIRS = app_templates file_templates doc


kde_module_LTLIBRARIES = libkdevadasupport.la
libkdevadasupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN)
libkdevadasupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la 				$(top_builddir)/lib/antlr/src/libantlr.la

libkdevadasupport_la_SOURCES = adasupportpart.cpp problemreporter.cpp backgroundparser.cpp addclassdlg.ui addclass.cpp configproblemreporter.ui ada_utils.cpp adasupport.cpp AdaLexer.cpp AdaParser.cpp AdaTreeParserSuper.cpp AdaStoreWalker.cpp

METASOURCES = AUTO

EXTRA_DIST = ada.g ada.tree.g ada.store.g

## The following three rules assume that you have Java installed,
## ANTLR installed, and you have the antlr jar in your CLASSPATH.

#AdaLexer.hpp AdaLexer.cpp AdaParser.hpp AdaParser.cpp: ada.g
#	antlr   ada.g

#AdaTreeParserSuper.hpp AdaTreeParserSuper.cpp: ada.tree.g
#	antlr   ada.tree.g

#AdaStoreWalker.hpp AdaStoreWalker.cpp: ada.store.g
#	antlr -glib ada.tree.g  ada.store.g

genparser:
	antlr   ada.g && antlr   ada.tree.g && antlr -glib ada.tree.g  ada.store.g

servicedir = $(kde_servicesdir)
service_DATA = kdevadasupport.desktop

rcdir = $(kde_datadir)/kdevadasupport
rc_DATA = kdevadasupport.rc

KDE_OPTIONS = nofinal

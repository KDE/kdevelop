# Here resides the Java support part.
KDE_CXXFLAGS = $(USE_EXCEPTIONS)

INCLUDES = 	-I$(top_srcdir)/lib/antlr \
		-I$(top_srcdir)/lib/interfaces \
		-I$(top_srcdir)/lib/sourceinfo \
		$(all_includes)

kde_module_LTLIBRARIES = libkdevjavasupport.la
libkdevjavasupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN) 
libkdevjavasupport_la_LIBADD = 	$(top_builddir)/lib/libkdevelop.la \
				$(top_builddir)/lib/antlr/src/libantlr.la

libkdevjavasupport_la_SOURCES = javasupportpart.cpp \
				addclassdlg.ui addclass.cpp \
				JavaLexer.cpp JavaRecognizer.cpp \
				JavaStoreWalker.cpp problemreporter.cpp 

METASOURCES = AUTO

EXTRA_DIST = java.g java.tree.g java.store.g

#DISTCLEANFILES =	JavaLexer.cpp JavaLexer.hpp \
#			JavaRecognizer.cpp JavaRecognizer.hpp \
#			JavaStoreWalker.cpp JavaStoreWalker.hpp
#
#JavaLexer.hpp JavaLexer.cpp JavaRecognizer.hpp JavaRecognizer.cpp: java.g
#	antlr $<
#
#JavaStoreWalker.hpp JavaStoreWalker.cpp: java.store.g
#	antlr $<

servicedir = $(kde_servicesdir)
service_DATA = kdevjavasupport.desktop



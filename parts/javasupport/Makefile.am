# Here resides the Java support part.

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo  $(jni_includes) $(all_includes)

kde_module_LTLIBRARIES = libkdevjavasupport.la
libkdevjavasupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN) 
libkdevjavasupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la 

libkdevjavasupport_la_SOURCES = javasupportpart.cpp javaclassparser.cpp tokenizer.cpp \
				addclassdlg.ui addclass.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevjavasupport.desktop

EXTRA_DIST = tokenizer.l
tokenizer.cpp: tokenizer.l
	flex -+ -otokenizer.cpp $(srcdir)/tokenizer.l
		

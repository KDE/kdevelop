# Here resides the Java support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(all_includes)

kde_module_LTLIBRARIES = libkdevjavasupport.la
libkdevjavasupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevjavasupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevjavasupport_la_SOURCES = javasupportfactory.cpp \
			       javasupportpart.cpp \
			       classparser.cpp \
			       tokenizer.cpp \
			       javaaddclassmethoddlg.cpp \
			       javaaddclassattributedlg.cpp

EXTRA_DIST = tokenizer.l
tokenizer.cpp: tokenizer.l
	$(LEX) -+ -otokenizer.cpp $(srcdir)/tokenizer.l

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevjavasupport.desktop

# Here resides the Java support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(all_includes)

lib_LTLIBRARIES = libkdevjavasupport.la
libkdevjavasupport_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevjavasupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la -lfl

libkdevjavasupport_la_SOURCES = javasupportfactory.cpp \
			       javasupportpart.cpp \
			       classparser.cpp \
			       tokenizer.cpp \
			       javaaddclassmethoddlg.cpp \
			       javaaddclassattributedlg.cpp

EXTRA_DIST = tokenizer.l
tokenizer.cc: tokenizer.l
	$(LEX) -+ -otokenizer.cpp $(srcdir)/tokenizer.l

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevjavasupport.desktop

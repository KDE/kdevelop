# Here resides the C/C++ support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(all_includes)

kde_module_LTLIBRARIES = libkdevcppsupport.la
libkdevcppsupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN)
libkdevcppsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevcppsupport_la_SOURCES = cppsupportfactory.cpp \
			       cppsupportpart.cpp \
			       classparser.cpp \
			       tokenizer.cpp \
			       addclassmethoddlg.cpp \
			       addclassattributedlg.cpp

EXTRA_DIST = tokenizer.l
tokenizer.cpp: tokenizer.l
	flex -+ -otokenizer.cpp $(srcdir)/tokenizer.l

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop kdevcsupport.desktop

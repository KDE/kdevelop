# Here resides the C/C++ support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(all_includes)

lib_LTLIBRARIES = libkdevcppsupport.la
libkdevcppsupport_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevcppsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la -lfl

libkdevcppsupport_la_SOURCES = cppsupportfactory.cpp \
			       cppsupportpart.cpp \
			       classparser.cpp \
			       tokenizer.cpp \
			       addclassmethoddlg.cpp \
			       addclassattributedlg.cpp

EXTRA_DIST = tokenizer.l
tokenizer.cc: tokenizer.l
	$(LEX) -+ -otokenizer.cpp $(srcdir)/tokenizer.l

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop kdevcsupport.desktop

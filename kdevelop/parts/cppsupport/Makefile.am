# Here resides the class view part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib/projectmanagement -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/lib/sourceinfo $(all_includes)

lib_LTLIBRARIES = libkdevcppsupport.la
libkdevcppsupport_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevcppsupport_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la -lfl

libkdevcppsupport_la_SOURCES = main.cpp cppsupport.cpp classparser.cpp tokenizer.cpp\
			       caddclassmethoddlg.cpp caddclassattributedlg.cpp

noinst_HEADERS = main.h cppsupport.h classparser.h tokenizer.h \
		 caddclassmethoddlg.h caddclassattributedlg.h

EXTRA_DIST = tokenizer.l
tokenizer.cc: tokenizer.l
	$(LEX) -+ -otokenizer.cpp $(srcdir)/tokenizer.l

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop

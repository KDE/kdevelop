# Here resides the class view part.

INCLUDES = 	-I$(top_srcdir)/kdevelop/lib/projectmanagement \
						-I$(top_srcdir)/kdevelop/lib/general \
						-I$(top_srcdir)/kdevelop/lib/ComponentManager \
					 	-I$(top_srcdir)/kdevelop/lib/ComponentManager/Components \
						-I$(top_srcdir)/kdevelop/parts/SourceInfo \
						$(all_includes)

lib_LTLIBRARIES = libkdevjavasupport.la
libkdevjavasupport_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevjavasupport_la_LIBADD = $(top_builddir)/kdevelop/parts/SourceInfo/libkdevsourceinfo.la $(top_builddir)/kdevelop/lib/libkdevelop.la -lfl

libkdevjavasupport_la_SOURCES = main.cpp javasupport.cpp classparser.cpp tokenizer.cpp\
			       javaaddclassmethoddlg.cpp javaaddclassattributedlg.cpp

noinst_HEADERS = main.h javasupport.h classparser.h tokenizer.h \
		 javaaddclassmethoddlg.h javaaddclassattributedlg.h

EXTRA_DIST = tokenizer.l
tokenizer.cpp: tokenizer.l
	$(LEX) -+ -otokenizer.cpp $(srcdir)/tokenizer.l

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevjavasupport.desktop
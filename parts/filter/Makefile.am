# Here resides the filter part

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	   -I$(top_srcdir)/lib/util \
	   $(all_includes)

kde_module_LTLIBRARIES = libkdevfilter.la
libkdevfilter_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevfilter_la_LIBADD = $(top_builddir)/lib/libkdevelop.la -lktexteditor 

libkdevfilter_la_SOURCES = filterpart.cpp \
			   shellfilterdlg.cpp \
			   shellinsertdlg.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevfilter.desktop

rcdir = $(kde_datadir)/kdevfilter
rc_DATA = kdevfilter.rc

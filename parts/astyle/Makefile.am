INCLUDES = -I$(top_srcdir)/lib/astyle -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions $(all_includes)

kde_module_LTLIBRARIES = libkdevastyle.la
libkdevastyle_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevastyle_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/astyle/libastyle.la 

libkdevastyle_la_SOURCES = astyle_part.cpp astyleconfig.ui astyle_widget.cpp astyle_adaptor.cpp
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevastyle.desktop

rc_DATA = kdevpart_astyle.rc
rcdir = $(kde_datadir)/kdevastyle

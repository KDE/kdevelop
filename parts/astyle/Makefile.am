INCLUDES = $(all_includes) -I$(top_srcdir)/lib/interfaces

kde_module_LTLIBRARIES = libkdevastyle.la
libkdevastyle_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevastyle_la_LIBADD = $(top_builddir)/lib/libkdevelop.la -lktexteditor

libkdevastyle_la_SOURCES = astyle_part.cpp ASBeautifier.cpp ASFormatter.cpp ASResource.cpp astyleconfig.ui astyle_widget.cpp astyle_adaptor.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_astyle.desktop

rc_DATA = kdevpart_astyle.rc
rcdir = $(kde_datadir)/kdevastyle

# Here resides the custom project part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/external_interfaces	   -I$(top_srcdir)/lib/project -I$(top_builddir)/lib/project 	   -I$(top_srcdir)/lib/util 	   $(all_includes)

kde_module_LTLIBRARIES = libkdevcustomproject.la
libkdevcustomproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcustomproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevcustomproject_la_SOURCES = customprojectpart.cpp custombuildoptionswidgetbase.ui custombuildoptionswidget.cpp custommakeconfigwidgetbase.ui custommakeconfigwidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcustomproject.desktop

rcdir = $(kde_datadir)/kdevcustomproject
rc_DATA = kdevcustomproject.rc

# Here resides the ant project part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/external_interfaces	   -I$(top_srcdir)/lib/project -I$(top_builddir)/lib/project 	   -I$(top_srcdir)/lib/util 	   $(all_includes)

kde_module_LTLIBRARIES = libkdevantproject.la
libkdevantproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevantproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevantproject_la_SOURCES = antprojectpart.cpp antoptionswidget.ui classpathwidget.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevantproject.desktop

rcdir = $(kde_datadir)/kdevantproject
rc_DATA = kdevantproject.rc

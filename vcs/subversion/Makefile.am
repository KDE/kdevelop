INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/interfaces/extensions $(all_includes)

kde_module_LTLIBRARIES = libkdevsubversion.la
libkdevsubversion_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevsubversion_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/extensions/libkdevextensions.la

libkdevsubversion_la_SOURCES = subversion_core.skel subversion_part.cpp subversion_widget.cpp subversion_core.cpp svn_co.ui subversionprojectwidget.ui subversion_fileinfo.cpp subversion_fileinfo.skel subversiondiff.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevsubversion.desktop

rcdir = $(kde_datadir)/kdevsubversion
SUBDIRS = integrator

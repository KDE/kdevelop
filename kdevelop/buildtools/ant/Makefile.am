# Here resides the ant project part.

INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_builddir)/buildtools/lib/widgets \
	$(all_includes)

kde_module_LTLIBRARIES = libkdevantproject.la
libkdevantproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevantproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la $(top_builddir)/buildtools/lib/base/libkdevbuildbase.la

libkdevantproject_la_SOURCES = antprojectpart.cpp antoptionswidget.ui classpathwidget.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevantproject.desktop

rcdir = $(kde_datadir)/kdevantproject
rc_DATA = kdevantproject.rc

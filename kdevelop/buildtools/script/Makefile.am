# Here resides the script project part.
# This is a generic part for various scripting languages that simply 
# includes all files from the project directory without managing
# them. If a language needs more specific features, it deserves 
# its own part (or the respective features should be added to the
# language support part)

INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/external_interfaces $(all_includes)

kde_module_LTLIBRARIES = libkdevscriptproject.la
libkdevscriptproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevscriptproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la $(top_builddir)/buildtools/lib/base/libkdevbuildbase.la

libkdevscriptproject_la_SOURCES = scriptprojectpart.cpp scriptoptionswidget.cpp scriptoptionswidgetbase.ui scriptnewfiledlg.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevscriptproject.desktop

rcdir = $(kde_datadir)/kdevscriptproject
rc_DATA = kdevscriptproject.rc

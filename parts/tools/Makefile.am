INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevtools.la
libkdevtools_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevtools_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevtools_la_SOURCES = tools_part.cpp toolsconfig.cpp treeview.cpp \
			  toolsconfigwidgetbase.ui \
			  toolsconfigwidget.cpp \
			  addtooldlg.ui addtooldlg.ui.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_tools.desktop

rc_DATA = kdevpart_tools.rc
rcdir = $(kde_datadir)/kdevtools

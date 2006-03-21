INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevvalgrind.la
libkdevvalgrind_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevvalgrind_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevvalgrind_la_SOURCES = dialog_widget.ui valgrind_part.cpp valgrind_widget.cpp valgrind_dialog.cpp valgrinditem.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevvalgrind.desktop

rcdir = $(kde_datadir)/kdevvalgrind
rc_DATA = kdevpart_valgrind.rc

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevdistpart.la
libkdevdistpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevdistpart_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevdistpart_la_SOURCES = distpart_ui.ui distpart_factory.cpp distpart_part.cpp distpart_widget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_distpart.desktop


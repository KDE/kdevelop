INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdeveditorchooser.la
libkdeveditorchooser_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdeveditorchooser_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdeveditorchooser_la_SOURCES = editorchooser_part.cpp editchooser.ui editorchooser_widget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdeveditorchooser.desktop


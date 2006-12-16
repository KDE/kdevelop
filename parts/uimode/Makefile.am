INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevuichooser.la
libkdevuichooser_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevuichooser_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevuichooser_la_SOURCES = uichooser_part.cpp uichooser.ui uichooser_widget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevuichooser.desktop


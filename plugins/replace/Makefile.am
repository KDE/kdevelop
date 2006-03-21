# Here resides the replace part (search and replace in the entire project)

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevreplace.la
libkdevreplace_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevreplace_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
libkdevreplace_la_SOURCES = replace_part.cpp replace_widget.cpp replacedlg.ui replaceitem.cpp replaceview.cpp replacedlgimpl.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevreplace.desktop

rcdir = $(kde_datadir)/kdevreplace
rc_DATA = kdevpart_replace.rc

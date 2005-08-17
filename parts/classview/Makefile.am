# Here resides the class view part.

INCLUDES = \
	-I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/widgets \
	$(all_includes)

kde_module_LTLIBRARIES = libkdevclassview.la
libkdevclassview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevclassview_la_LIBADD = \
  $(top_builddir)/lib/libkdevelop.la \
  $(top_builddir)/lib/widgets/libkdevwidgets.la \
  $(LIB_KHTML)

libkdevclassview_la_SOURCES = \
	kdevclassview.cpp kdevclassviewdelegate.cpp  kdevclassview_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevclassview.desktop

rcdir = $(kde_datadir)/kdevclassview
rc_DATA = kdevclassview.rc

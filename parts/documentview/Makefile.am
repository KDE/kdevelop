# Here resides the document view part.

INCLUDES = \
	-I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/widgets \
	$(all_includes)

kde_module_LTLIBRARIES = libkdevdocumentview.la
libkdevdocumentview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevdocumentview_la_LIBADD = \
  $(top_builddir)/lib/libkdevelop.la \
  $(top_builddir)/lib/widgets/libkdevwidgets.la \
  $(LIB_KHTML)

libkdevdocumentview_la_SOURCES = kdevdocumentview.cpp \
	kdevdocumentviewdelegate.cpp kdevdocumentview_part.cpp kdevdocumentmodel.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevdocumentview.desktop

rcdir = $(kde_datadir)/kdevdocumentview
rc_DATA = kdevdocumentview.rc

# Here resides the guibuilder part.

INCLUDES = \
  -I$(top_srcdir)/lib/interfaces \
  -I$(top_srcdir)/lib/projectmanager/interfaces \
  -I$(top_srcdir)/lib/util \
  $(all_includes)

kde_module_LTLIBRARIES = libkdevguibuilder.la
libkdevguibuilder_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevguibuilder_la_LIBADD = \
  $(top_builddir)/lib/libkdevelop.la \
  $(top_builddir)/lib/projectmanager/interfaces/libprojectmanager_interfaces.la  

libkdevguibuilder_la_SOURCES = \
  guibuilder_part.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevguibuilder.desktop


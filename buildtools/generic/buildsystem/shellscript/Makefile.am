INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/project -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets/propeditor $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = libkdevbuildscript.la
libkdevbuildscript_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
libkdevbuildscript_la_LIBADD = $(top_builddir)/lib/project/libkdevproject.la $(top_builddir)/lib/interfaces/libkdevinterfaces.la $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/util/libkdevutil.la
libkdevbuildscript_la_SOURCES =  shellscriptbuildsystem.cpp

noinst_HEADERS =  shellscriptbuildsystem.h

servicedir = $(kde_servicesdir)
service_DATA = kdevbuildscript.desktop

# Here resides the Python support part.
# Do not confuse this with the python part responsible for KDevelop scripting

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(all_includes)

kde_module_LTLIBRARIES = libkdevpythonsupport.la
libkdevpythonsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevpythonsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevpythonsupport_la_SOURCES = pythonsupportfactory.cpp \
			          pythonsupportpart.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpythonsupport.desktop

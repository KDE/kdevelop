# Here resides the python scripting part

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
	   $(all_includes) $(PYTHONINC)

kde_module_LTLIBRARIES = libkdevscripting.la
libkdevscripting_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevscripting_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(PYTHONLIB) $(LIBPYTHON)

libkdevscripting_la_SOURCES = scriptingpart.cpp \
			      pydcopc.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevscripting.desktop

scriptdir = $(kde_datadir)/kdevscripting
script_DATA = init.py kdevelop.py pydcop.py

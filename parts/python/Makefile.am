# Here resides the guile part.

# Hack
#PYTHONINC = -I/usr/include/python2.0
#PYTHONLIB = -L/usr/lib/python2.0/config -lpython2.0 -lpthread -ldb-3

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	   -I$(top_srcdir)/lib/util \
	   $(all_includes) \
	   $(PYTHONINC)

kde_module_LTLIBRARIES = libkdevpython.la
libkdevpython_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevpython_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(PYTHONLIB) $(LIBPYTHON)

libkdevpython_la_SOURCES = pythonfactory.cpp \
			   pythonpart.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpython.desktop

scriptdir = $(kde_datadir)/kdevpython
script_DATA = init.py gideon.py

# Here resides the Python support part.
# Do not confuse this with the python part responsible for KDevelop scripting

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/languages/lib/designer_integration \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util -I$(top_builddir)/languages/lib/designer_integration \
	$(all_includes)
SUBDIRS = app_templates file_templates doc


kde_module_LTLIBRARIES = libkdevpythonsupport.la kio_pydoc.la

libkdevpythonsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevpythonsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
libkdevpythonsupport_la_SOURCES = pythonsupportpart.cpp pythonconfigwidget.cpp pythonconfigwidgetbase.ui

kio_pydoc_la_SOURCES = pydoc.cpp
kio_pydoc_la_LIBADD = $(LIB_KIO)
kio_pydoc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

METASOURCES = AUTO

slavedir = $(kde_datadir)/kio_pydoc
slave_DATA = kde_pydoc.py kde_inspect.py

servicedir = $(kde_servicesdir)
service_DATA = kdevpythonsupport.desktop pydoc.protocol

rcdir = $(kde_datadir)/kdevpythonsupport
rc_DATA = kdevpythonsupport.rc

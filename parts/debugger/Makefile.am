# Here resides the debugger part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

lib_LTLIBRARIES = libkdevdebugger.la
libkdevdebugger_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevdebugger_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevdebugger_la_SOURCES = debuggerfactory.cpp \
			     debuggerpart.cpp

METASOURCES = AUTO

#servicedir = $(kde_servicesdir)
#service_DATA = kdevdebugger.desktop

rcdir = $(kde_datadir)/kdevdebugger
rc_DATA = kdevdebugger.rc

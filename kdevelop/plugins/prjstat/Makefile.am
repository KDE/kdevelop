
INCLUDES= $(KDE_INCLUDES) $(QT_INCLUDES) $(X_INCLUDES) -I$(top_srcdir)/kdevelop -I$(top_srcdir)/plugins -I$(srcdir)

lib_LTLIBRARIES  = libprjstat_kdevplg.la

SOMAJOR = 1
SOMINOR = 0


noinst_HEADERS = prjstatplugin.h

libprjstat_kdevplg_la_SOURCES = prjstatplugin.cpp main.cpp
libprjstat_kdevplg_la_METASOURCES = AUTO  

EXTRA_DIST =

LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS)
libprjstat_kdevplg_la_LDFLAGS = -version-info $(SOMAJOR):$(SOMINOR) $(LDFLAGS)


INCLUDES= $(KDE_INCLUDES) $(QT_INCLUDES) $(X_INCLUDES) -I../../ -I../ -I.

lib_LTLIBRARIES  = libprjstat_kdevplg.la

SOMAJOR = 1
SOMINOR = 0


noinst_HEADERS = prjstatplugin.h

libprjstat_kdevplg_la_SOURCES = prjstatplugin.cpp main.cpp
libprjstat_kdevplg_la_METASOURCES = USE_AUTOMOC  

EXTRA_DIST = README

LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS)
libprjstat_kdevplg_la_LDFLAGS = -version-info $(SOMAJOR):$(SOMINOR) $(LDFLAGS)

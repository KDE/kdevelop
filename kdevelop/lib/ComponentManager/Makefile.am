## set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I$(top_srcdir)/kdevelop/lib/general
noinst_LTLIBRARIES = libKDevComponentManager.la

libKDevComponentManager_la_SOURCES = KDevComponentManager.cpp partloader.cpp kdevcomponent.cpp
libKDevComponentManager_la_LIBADD = $(top_builddir)/kdevelop/lib/ComponentManager/Components/libKDevComponentManagerComponents.la
## noinst_HEADERS =

SUBDIRS = Components

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kdevelopcomponent.desktop


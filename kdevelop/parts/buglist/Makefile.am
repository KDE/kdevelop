# BugList Part

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/lib/projectmanagement $(all_includes)

lib_LTLIBRARIES = libkdevbuglist.la
libkdevbuglist_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevbuglist_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevbuglist_la_SOURCES = bugcounter.cpp bug.cpp bugedit.cpp \
    buglist.cpp main.cpp buglistcomponent.cpp

noinst_HEADERS = bugcounter.h bug.h bugedit.h buglist.h main.h \
    buglistcomponent.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevbuglist.desktop

rcdir = $(kde_datadir)/kdevbuglist
rc_DATA = kdevbuglist.rc



# Here resides the regexptest part

INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkdevregexptest.la
libkdevregexptest_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevregexptest_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevregexptest_la_SOURCES = regexptestpart.cpp regexptestdlg.cpp regexptestdlgbase.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevregexptest.desktop

rcdir = $(kde_datadir)/kdevregexptest
rc_DATA = kdevregexptest.rc

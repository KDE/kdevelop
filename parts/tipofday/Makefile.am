INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

lib_LTLIBRARIES = libkdevtipofday.la
libkdevtipofday_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevtipofday_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevtipofday_la_SOURCES = tipofday_factory.cpp tipofday_part.cpp tipofday.ui ktipdatabase.cpp

METASOURCES = AUTO
KDE_ICON = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_tipofday.desktop

tip_DATA = tips
tipdir = $(kde_datadir)/kdevtipofday


rc_DATA = kdevpart_tipofday.rc
rcdir = $(kde_datadir)/kdevtipofday

messages:
	perl ./preparetips > tips.cc
	$(XGETTEXT) *.cpp *.h tips.cc -o $(podir)/kdevtipofday.pot
	rm -f tips.cc

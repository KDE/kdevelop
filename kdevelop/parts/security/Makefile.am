INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
	$(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = libkdevsecurity.la
libkdevsecurity_la_LDFLAGS = -avoid-version $(all_libraries)
libkdevsecurity_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
noinst_HEADERS = securitypart.h securitywidget.h securitychecker.h
libkdevsecurity_la_SOURCES = securitypart.cpp securitywidget.cpp \
	securitychecker.cpp
servicedir = $(kde_servicesdir)
service_DATA = kdevsecurity.desktop
securitydatadir = $(kde_datadir)/kdevsecurity
securitydata_DATA = patterns

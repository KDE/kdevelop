INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = libdoccustomplugin.la
libdoccustomplugin_la_LDFLAGS = $(all_libraries)
libdoccustomplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la
noinst_HEADERS = doccustomplugin.h
libdoccustomplugin_la_SOURCES = doccustomplugin.cpp
kde_services_DATA = doccustomplugin.desktop

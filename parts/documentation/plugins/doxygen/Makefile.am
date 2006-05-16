INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = libdocdoxygenplugin.la

kde_services_DATA =  docdoxygenplugin.desktop
libdocdoxygenplugin_la_SOURCES = docdoxygenplugin.cpp
noinst_HEADERS =  docdoxygenplugin.h


libdocdoxygenplugin_la_LDFLAGS = -module -avoid-version -no-undefined $(all_libraries)
libdocdoxygenplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la

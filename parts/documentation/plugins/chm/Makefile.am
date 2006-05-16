INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES =  libdocchmplugin.la

noinst_HEADERS =  docchmplugin.h
kde_services_DATA =  docchmplugin.desktop

libdocchmplugin_la_LDFLAGS = -module -avoid-version -no-undefined $(all_libraries)
libdocchmplugin_la_SOURCES =  docchmplugin.cpp
libdocchmplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la

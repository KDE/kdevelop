INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES =  libdocdevhelpplugin.la

noinst_HEADERS =  docdevhelpplugin.h
kde_services_DATA =  docdevhelpplugin.desktop

libdocdevhelpplugin_la_LDFLAGS = -module -avoid-version -no-undefined $(all_libraries)
libdocdevhelpplugin_la_SOURCES =  docdevhelpplugin.cpp
libdocdevhelpplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES =  libdocchmplugin.la
libdocchmplugin_la_LDFLAGS = $(all_libraries)
kde_services_DATA =  docchmplugin.desktop
noinst_HEADERS =  docchmplugin.h
libdocchmplugin_la_SOURCES =  docchmplugin.cpp
libdocchmplugin_la_LIBADD =\
	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la

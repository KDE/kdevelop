INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)

METASOURCES = AUTO

kde_module_LTLIBRARIES =  libdocqtplugin.la

noinst_HEADERS = docqtplugin.h
kde_services_DATA =  docqtplugin.desktop

libdocqtplugin_la_LDFLAGS = -module -avoid-version -no-undefined $(all_libraries)
libdocqtplugin_la_SOURCES = docqtplugin.cpp
libdocqtplugin_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la

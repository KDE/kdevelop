INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES = libdocdoxygenplugin.la

kde_services_DATA =  docdoxygenplugin.desktop
libdocdoxygenplugin_la_SOURCES = docdoxygenplugin.cpp
noinst_HEADERS =  docdoxygenplugin.h
libdocdoxygenplugin_la_LIBADD = $(LIB_KHTML)
libdocdoxygenplugin_la_LDFLAGS = $(all_libraries)

INCLUDES = -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	$(all_includes)
METASOURCES = AUTO
libkdevextras_la_LDFLAGS = $(all_libraries)
noinst_LTLIBRARIES = libkdevextras.la
kdevelopincludedir = $(includedir)/kdevelop/interfaces/extras
servicetypedir = $(kde_servicetypesdir)

kdevelopinclude_HEADERS = kdevcompileroptions.h kdevvcsintegrator.h
libkdevextras_la_SOURCES = kdevcompileroptions.cpp kdevvcsintegrator.cpp
servicetype_DATA = kdevelopcompileroptions.desktop \
	kdevelopvcsintegrator.desktop


INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions $(all_includes)
METASOURCES = AUTO

noinst_LTLIBRARIES = libkdevdcopextensions.la
libkdevdcopextensions_la_LDFLAGS = $(all_libraries)
libkdevdcopextensions_la_SOURCES = KDevAppFrontendIface.cpp \
	KDevMakeFrontendIface.cpp KDevAppFrontendIface.skel KDevMakeFrontendIface.skel

dcopincludeextdir = $(includedir)/kdevelop/interfaces/extensions/dcop
dcopincludeext_HEADERS = KDevAppFrontendIface.h KDevMakeFrontendIface.h

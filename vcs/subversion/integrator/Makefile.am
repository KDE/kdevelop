INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util \
	$(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = libsubversionintegrator.la
libsubversionintegrator_la_LDFLAGS = $(all_libraries)
libsubversionintegrator_la_LIBADD =\
	$(top_builddir)/lib/interfaces/extras/libkdevextras.la\
	$(top_builddir)/lib/libkdevelop.la
kde_services_DATA = kdevsubversionintegrator.desktop
noinst_HEADERS = subversionintegrator.h svnintegratordlg.h
libsubversionintegrator_la_SOURCES = subversionintegrator.cpp \
	svnintegratordlgbase.ui svnintegratordlg.cpp

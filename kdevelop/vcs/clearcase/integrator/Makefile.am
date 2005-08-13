INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util \
	$(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = libclearcaseintegrator.la
libclearcaseintegrator_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
libclearcaseintegrator_la_LIBADD =\
	$(top_builddir)/lib/interfaces/extras/libkdevextras.la\
	$(top_builddir)/lib/libkdevelop.la
kde_services_DATA = kdevclearcaseintegrator.desktop
noinst_HEADERS = clearcaseintegrator.h ccintegratordlg.h
libclearcaseintegrator_la_SOURCES = clearcaseintegrator.cpp \
	ccintegratordlgbase.ui ccintegratordlg.cpp

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util \
	$(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = libperforceintegrator.la
libperforceintegrator_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
libperforceintegrator_la_LIBADD =\
	$(top_builddir)/lib/interfaces/extras/libkdevextras.la\
	$(top_builddir)/lib/libkdevelop.la
kde_services_DATA = kdevperforceintegrator.desktop
noinst_HEADERS = perforceintegrator.h pfintegratordlg.h
libperforceintegrator_la_SOURCES = perforceintegrator.cpp \
	pfintegratordlgbase.ui pfintegratordlg.cpp

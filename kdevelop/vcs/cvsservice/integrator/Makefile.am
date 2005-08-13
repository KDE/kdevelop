INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util \
	$(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = libcvsserviceintegrator.la
libcvsserviceintegrator_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
libcvsserviceintegrator_la_LIBADD = \
	$(top_builddir)/lib/interfaces/extras/libkdevextras.la $(top_builddir)/lib/libkdevelop.la -lcvsservice
noinst_HEADERS = cvsserviceintegrator.h integratordlg.h
libcvsserviceintegrator_la_SOURCES = cvsserviceintegrator.cpp \
	integratordlgbase.ui fetcherdlgbase.ui integratordlg.cpp initdlg.ui
kde_services_DATA = kdevcvsserviceintegrator.desktop

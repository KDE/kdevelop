# Here resides the cvs part

INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkdevcvs.la
libkdevcvs_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcvs_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevcvs_la_SOURCES = cvspart.cpp \
			commitdlg.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcvs.desktop

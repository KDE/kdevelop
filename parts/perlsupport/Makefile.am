# Here resides the Perl support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(all_includes)

lib_LTLIBRARIES = libkdevperlsupport.la
libkdevperlsupport_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevperlsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevperlsupport_la_SOURCES = perlsupportfactory.cpp \
				perlsupportpart.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevperlsupport.desktop

# Here resides the cvs part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/lib/projectmanagement \
					 -I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler $(all_includes)

lib_LTLIBRARIES = libkdevcvsinterface.la
libkdevcvsinterface_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevcvsinterface_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevcvsinterface_la_SOURCES = main.cpp cvsinterface.cpp commitdialog.cpp

noinst_HEADERS = main.h cvsinterface.h commitdialog.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcvsinterface.desktop

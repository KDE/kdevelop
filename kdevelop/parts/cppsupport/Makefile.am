# Here resides the class view part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib -I$(top_srcdir)/kdevelop/classparser $(all_includes)

lib_LTLIBRARIES = libkdevcppsupport.la
libkdevcppsupport_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevcppsupport_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la \
			     $(top_builddir)/kdevelop/classparser/libkdevclassparser.la $(LIB_KDEUI)

libkdevcppsupport_la_SOURCES = main.cpp cppsupport.cpp caddclassmethoddlg.cpp caddclassattributedlg.cpp

noinst_HEADERS = main.h cppsupport.h caddclassmethoddlg.h caddclassattributedlg.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop

# Here resides the class view part.

INCLUDES = -I$(top_srcdir)/kdevelop/lib -I$(top_srcdir)/kdevelop/classparser $(all_includes)

lib_LTLIBRARIES = libkdevclassview.la
libkdevclassview_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevclassview_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la \
			     $(top_builddir)/kdevelop/classparser/libkdevclassparser.la $(LIB_KDEUI)

libkdevclassview_la_SOURCES = main.cpp classview.cpp classactions.cpp caddclassmethoddlg.cpp caddclassattributedlg.cpp

noinst_HEADERS = main.h classview.h classactions.h caddclassmethoddlg.h caddclassattributedlg.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevclassview.desktop

rcdir = $(kde_datadir)/kdevclassview
rc_DATA = kdevclassview.rc

# Here resides the print plugin.

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general $(all_includes)

lib_LTLIBRARIES = libkdevprintplugin.la
libkdevprintplugin_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevprintplugin_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevprintplugin_la_SOURCES = main.cpp cfileprintdlg.cpp cprintdlg.cpp \
			        cconfigenscriptdlg.cpp cconfiga2psdlg.cpp cdatepikerdlg.cpp

noinst_HEADERS = main.h cprintdlg.h cfileprintdlg.h \
		 cconfigenscriptdlg.h cconfiga2psdlg.h cdatepikerdlg.h

METASOURCES = AUTO

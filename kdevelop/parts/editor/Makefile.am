# Here resides the editor part.
INCLUDES = -I$(top_srcdir)/kdevelop/lib/general $(all_includes)
lib_LTLIBRARIES = libkdeveditorpart.la

libkdeveditorpart_la_LDFLAGS =  \
    $(all_libraries) $(KDE_PLUGIN)

libkdeveditorpart_la_LIBADD = \
    $(top_builddir)/kdevelop/lib/libkdevelop.la \
    $(LIB_KDEUI)

libkdeveditorpart_la_SOURCES = \
    editorfactory.cpp editormgr.cpp editorconfigwidget.cpp editorview.cpp

noinst_HEADERS = \
    editorfactory.h editormgr.h editorconfigwidget.h editorview.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdeveditorpart.desktop

rcdir = $(kde_datadir)/kdeveditorpart
rc_DATA = kdeveditorpartui.rc

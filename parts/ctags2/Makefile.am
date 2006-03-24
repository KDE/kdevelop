INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevctags2.la
libkdevctags2_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevctags2_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevctags2_la_SOURCES = ctags2_part.cpp readtags.c ctags2_widgetbase.ui \
	ctags2_widget.cpp tags.cpp ctagskinds.cpp ctags2_settingswidgetbase.ui \
	ctags2_settingswidget.cpp ctags2_selecttagfilebase.ui ctags2_selecttagfile.cpp \
	ctags2_createtagfilebase.ui ctags2_createtagfile.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevctags2.desktop

rc_DATA = kdevpart_ctags2.rc
rcdir = $(kde_datadir)/kdevctags2

noinst_HEADERS = ctags2_widget.h tags.h readtags.h ctagskinds.h \
	ctags2_settingswidget.h

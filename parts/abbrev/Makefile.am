# Here resides the abbrev part

INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

kde_module_LTLIBRARIES = libkdevabbrev.la
libkdevabbrev_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevabbrev_la_LIBADD = $(top_builddir)/lib/libkdevelop.la -lktexteditor

libkdevabbrev_la_SOURCES = abbrevpart.cpp \
			   abbrevconfigwidget.cpp \
			   abbrevconfigwidgetbase.ui \
			   addtemplatedlg.cpp \
			   addtemplatedlgbase.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevabbrev.desktop

rcdir = $(kde_datadir)/kdevabbrev
rc_DATA = kdevabbrev.rc

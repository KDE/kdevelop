# Here resides the Perl support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevphpsupport.la
libkdevphpsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevphpsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(LIB_KHTML)

libkdevphpsupport_la_SOURCES = phpsupportfactory.cpp \
				phpsupportpart.cpp phpconfigwidgetbase.ui \
				phpconfigwidget.cpp phpinfodlg.ui phphtmlview.cpp \
				phperrorview.cpp phpconfigdata.cpp phpbookconfigwidget.ui \
				phpbookconfig.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevphpsupport.desktop

rcdir = $(kde_datadir)/kdevphpsupport
rc_DATA = kdevphpsupport.rc

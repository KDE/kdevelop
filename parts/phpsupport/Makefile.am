# Here resides the PHP support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevphpsupport.la
libkdevphpsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevphpsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(LIB_KHTML) -lktexteditor

libkdevphpsupport_la_SOURCES = phpsupportpart.cpp phpconfigwidgetbase.ui phpconfigparserwidgetbase.ui \
				phpconfigwidget.cpp phpinfodlg.ui phphtmlview.cpp phpconfigparserwidget.cpp \
				phperrorview.cpp phpconfigdata.cpp \
				phpcodecompletion.cpp phpparser.cpp \
				phpnewclassdlgbase.ui phpnewclassdlg.cpp

METASOURCES = AUTO
SUBDIRS = data
servicedir = $(kde_servicesdir)
service_DATA = kdevphpsupport.desktop

rcdir = $(kde_datadir)/kdevphpsupport
rc_DATA = kdevphpsupport.rc

templatedir = ${kde_datadir}/kdevabbrev/templates
template_DATA = phptemplates

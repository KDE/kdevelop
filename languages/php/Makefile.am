# Here resides the PHP support part.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/widgets $(all_includes)
SUBDIRS = data app_templates file_templates doc


kde_module_LTLIBRARIES = libkdevphpsupport.la
libkdevphpsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevphpsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libwidgets.la $(LIB_KHTML)

libkdevphpsupport_la_SOURCES = phpsupportpart.cpp phpconfigwidgetbase.ui phpconfigparserwidgetbase.ui phpconfigwidget.cpp phpinfodlg.ui phphtmlview.cpp phpconfigparserwidget.cpp phperrorview.cpp phpconfigdata.cpp phpcodecompletion.cpp phpparser.cpp phpnewclassdlgbase.ui phpnewclassdlg.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevphpsupport.desktop

rcdir = $(kde_datadir)/kdevphpsupport
rc_DATA = kdevphpsupport.rc

templatedir = ${kde_datadir}/kdevabbrev/templates
template_DATA = phptemplates

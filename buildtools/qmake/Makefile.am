# Here resides the troll project part for tmake or qmake


INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/buildtools/lib/parsers/qmake \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	-I$(top_builddir)/buildtools/lib/widgets $(all_includes)


kde_module_LTLIBRARIES = libkdevtrollproject.la
libkdevtrollproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevtrollproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/util/libkdevutil.la $(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la \
	$(top_builddir)/buildtools/lib/base/libkdevbuildbase.la \
	$(top_builddir)/buildtools/lib/parsers/qmake/libkdevqmakeparser.la

libkdevtrollproject_la_SOURCES = choosesubprojectdlg.cpp \
	choosesubprojectdlgbase.ui createscopedlg.cpp createscopedlgbase.ui disablesubprojectdlg.cpp \
	disablesubprojectdlgbase.ui newwidgetdlg.cpp newwidgetdlgbase.ui pathutil.cpp \
	projectconfigurationdlg.cpp projectconfigurationdlgbase.ui qmakedefaultopts.cpp \
	qmakeoptionswidget.cpp qmakeoptionswidgetbase.ui qmakescopeitem.cpp scope.cpp \
	trolllistview.cpp trollprojectpart.cpp trollprojectwidget.cpp


METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevtrollproject.desktop kdevtmakeproject.desktop

rcdir = $(kde_datadir)/kdevtrollproject
rc_DATA = kdevtrollproject.rc
noinst_HEADERS = createscopedlg.h disablesubprojectdlg.h qmakedefaultopts.h \
	qmakeoptionswidget.h qmakescopeitem.h trolllistview.h

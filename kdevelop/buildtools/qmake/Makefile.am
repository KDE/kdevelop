# Here resides the troll project part for tmake or qmake



INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	-I$(top_builddir)/buildtools/lib/widgets $(all_includes)


kde_module_LTLIBRARIES = libkdevtrollproject.la
libkdevtrollproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevtrollproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/util/libkdevutil.la $(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la \
	$(top_builddir)/buildtools/lib/base/libkdevbuildbase.la

libkdevtrollproject_la_SOURCES = caret.cpp filebuffer.cpp \
	projectconfigurationdlgbase.ui filepropertybase.ui newwidgetdlgbase.ui newwidgetdlg.cpp \
	filepropertydlg.cpp projectconfigurationdlg.cpp trollprojectpart.cpp trollprojectwidget.cpp \
	choosesubprojectdlgbase.ui choosesubprojectdlg.cpp trolllistview.cpp pathutil.cpp


METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevtrollproject.desktop kdevtmakeproject.desktop

rcdir = $(kde_datadir)/kdevtrollproject
rc_DATA = kdevtrollproject.rc
noinst_HEADERS = trolllistview.h

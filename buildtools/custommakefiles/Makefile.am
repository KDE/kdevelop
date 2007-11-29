# Here resides the custom project part.

INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes) \
	-I$(top_builddir)/buildtools/lib/widgets

kde_module_LTLIBRARIES = libkdevcustomproject.la
libkdevcustomproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcustomproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la $(top_builddir)/buildtools/lib/base/libkdevbuildbase.la

libkdevcustomproject_la_SOURCES = selectnewfilesdialog.cpp selectnewfilesdialogbase.ui \
	custombuildoptionswidget.cpp custombuildoptionswidgetbase.ui custommakeconfigwidget.cpp \
	custommakeconfigwidgetbase.ui custommanagerwidget.cpp custommanagerwidgetbase.ui \
	customotherconfigwidget.cpp customotherconfigwidgetbase.ui customprojectpart.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcustomproject.desktop

rcdir = $(kde_datadir)/kdevcustomproject
rc_DATA = kdevcustomproject.rc
noinst_HEADERS = selectnewfilesdialog.h custommanagerwidget.h \
	customotherconfigwidget.h

# Here resides the app wizard part.

SUBDIRS = common kapp kpartapp kpartplugin kappjava kcmodule kicker kioslave \
	  kdevpart pythonhello phphello chello cpphello gnomeapp

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(all_includes)

lib_LTLIBRARIES = libkdevappwizard.la
libkdevappwizard_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevappwizard_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevappwizard_la_SOURCES = appwizardfactory.cpp \
			      appwizardpart.cpp \
			      appwizarddlg.cpp \
				filepropspagebase.ui \
				filepropspage.cpp \
				appwizarddlgbase.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevappwizard.desktop

rcdir = $(kde_datadir)/kdevappwizard
rc_DATA = kdevappwizard.rc

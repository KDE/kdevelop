# Here resides the app wizard part.

SUBDIRS = common kapp kpartapp kpartplugin kappjava kdevpart

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(all_includes)

lib_LTLIBRARIES = libkdevappwizard.la
libkdevappwizard_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevappwizard_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevappwizard_la_SOURCES = appwizardfactory.cpp \
			      appwizardpart.cpp \
			      appwizarddlg.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevappwizard.desktop

rcdir = $(kde_datadir)/gideon
rc_DATA = kdevappwizard.rc

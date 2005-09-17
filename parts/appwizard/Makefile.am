
# Here resides the app wizard part.

SUBDIRS = . common licenses
INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevappwizard.la
libkdevappwizard_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevappwizard_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/util/libkdevutil.la

libkdevappwizard_la_SOURCES = appwizardfactory.cpp appwizardpart.cpp \
		appwizarddlg.cpp appwizarddlgbase.ui misc.cpp \
		kdevfile.cpp kdevlicense.cpp profilesupport.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevappwizard.desktop

rcdir = $(kde_datadir)/kdevappwizard
rc_DATA = kdevappwizard.rc


# Here resides the app wizard part.

SUBDIRS = . imports common licenses
INCLUDES = -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets $(all_includes)

kde_module_LTLIBRARIES = libkdevappwizard.la
libkdevappwizard_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevappwizard_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/interfaces/extras/libkdevextras.la

libkdevappwizard_la_SOURCES = appwizardfactory.cpp appwizardpart.cpp \
		appwizarddlg.cpp importdlg.cpp filepropspagebase.ui filepropspage.cpp \
		appwizarddlgbase.ui importdlgbase.ui misc.cpp vcs_form.ui autoform.cpp dataform.cpp \
		kdevfile.cpp kdevlicense.cpp profilesupport.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevappwizard.desktop

rcdir = $(kde_datadir)/kdevappwizard
rc_DATA = kdevappwizard.rc
noinst_HEADERS = autoform.h dataform.h profilesupport.h

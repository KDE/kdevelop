
# Here resides the app wizard part.

SUBDIRS = . imports common licenses
INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/widgets -I$(top_srcdir)/lib/sourceinfo $(all_includes)

kde_module_LTLIBRARIES = libkdevappwizard.la
libkdevappwizard_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevappwizard_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevappwizard_la_SOURCES = appwizardfactory.cpp appwizardpart.cpp appwizarddlg.cpp importdlg.cpp filepropspagebase.ui filepropspage.cpp appwizarddlgbase.ui importdlgbase.ui misc.cpp vcs_form.ui props_form.ui autoform.cpp dataform.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevappwizard.desktop

rcdir = $(kde_datadir)/kdevappwizard
rc_DATA = kdevappwizard.rc
noinst_HEADERS =  autoform.h dataform.h

# some directories
appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kfileplugindir = ${appwizarddatadir}/template-kfileplugin
templatedir = ${appwizarddatadir}/templates

# all kfileplugin data files
kfileplugin_DATA = README.devel kfile_plugin.cpp kfile_plugin.desktop kfile_plugin.h plugin.kdevelop kfileplugin preview.png src-Makefile.am subdirs
template_DATA = kfileplugin

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kfileplugindir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kfileplugindir)/script

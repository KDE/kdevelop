appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kpartplugindir = ${appwizarddatadir}/template-kpartplugin
templatedir = ${appwizarddatadir}/templates

kpartplugin_DATA = src-Makefile.am plugin_app.cpp plugin_app.h \
                   plugin_app.rc hi16-action-plugin.png hi22-action-plugin.png \
                   kpartplugin.png plugin.kdevelop subdirs
template_DATA = kpartplugin

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kpartplugindir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kpartplugindir)/script
	rm script.local

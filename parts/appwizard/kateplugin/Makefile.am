appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kateplugindir = ${appwizarddatadir}/template-kateplugin
templatedir = ${appwizarddatadir}/templates

kateplugin_DATA = src-Makefile.am plugin_app.cpp plugin_app.h \
                   plugin_app.rc hi16-action-plugin.png hi22-action-plugin.png \
                   kateplugin.png plugin.kdevelop subdirs plugin.desktop
template_DATA = kateplugin

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kateplugindir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kateplugindir)/script

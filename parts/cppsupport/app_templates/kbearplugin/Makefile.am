appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kbearplugindir = ${appwizarddatadir}/template-kbearplugin
templatedir = ${appwizarddatadir}/templates

kbearplugin_DATA = kbearplugin.desktop subdirs hi32-action-plugin.png outputwidget.cpp plugin.h script configwidget.cpp outputwidget.h plugin.rc src-Makefile.am Makefile.am kbearplugin-configure.in.in configwidget.h plugin.cpp plugin.kdevelop preview.png
template_DATA = kbearplugin

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kbearplugindir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kbearplugindir)/script

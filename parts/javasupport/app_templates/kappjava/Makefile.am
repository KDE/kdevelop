appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kappjavadir = ${appwizarddatadir}/template-kappjava
templatedir = ${appwizarddatadir}/templates

kappjava_DATA = app.java pref.java appview.java                 app_client.java appui.rc src-Makefile.am                 kappjava.png app.kdevelop subdirs
template_DATA = kappjava

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kappjavadir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kappjavadir)/script

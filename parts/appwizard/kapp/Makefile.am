appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kappdir = ${appwizarddatadir}/template-kapp
templatedir = ${appwizarddatadir}/templates

kapp_DATA = app.cpp app.h pref.cpp pref.h appview.cpp appview.h \
            appiface.h app_client.cpp main.cpp appui.rc src-Makefile.am \
	    kapp.png app.kdevelop subdirs README
template_DATA = kapp

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kappdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kappdir)/script


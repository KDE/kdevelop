appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kappdir = ${appwizarddatadir}/template-kapp
templatedir = ${appwizarddatadir}/templates

kapp_DATA = app.cpp app.h pref.cpp pref.h appview.cpp appview.h \
            appiface.h app_client.cpp main.cpp appui.rc src-Makefile.am \
	    kapp.png app.kdevelop subdirs
template_DATA = kapp

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(kappdir)/script


gideondatadir = ${kde_datadir}/gideon
commondatadir = ${gideondatadir}/template-common

kappjavadatadir = ${kde_datadir}/gideon/template-kappjava
kappjavadata_DATA = app.java pref.java appview.java \
		app_client.java appui.rc src-Makefile.am

templatedatadir = ${gideondatadir}/templates
templatedata_DATA = kappjava

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(kappjavadatadir)/script


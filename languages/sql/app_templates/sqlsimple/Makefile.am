appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
sqlsimpledir = ${appwizarddatadir}/template-sqlsimple
templatedir = ${appwizarddatadir}/templates

sqlsimple_DATA = app.sql app.kdevelop
template_DATA = sqlsimple

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(sqlsimpledir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(sqlsimpledir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
phphellodir = ${appwizarddatadir}/template-phphello
templatedir = ${appwizarddatadir}/templates

phphello_DATA = app.php app.kdevelop script
template_DATA = phphello

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(phphellodir)/script

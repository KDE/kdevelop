gideondatadir = ${kde_datadir}/gideon
commondatadir = ${gideondatadir}/template-common

phphellodatadir = ${kde_datadir}/gideon/template-phphello
phphellodata_DATA = app.php app.kdevelop script

templatedatadir = ${gideondatadir}/templates
templatedata_DATA = phphello

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(phphellodatadir)/script


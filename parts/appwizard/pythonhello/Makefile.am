gideondatadir = ${kde_datadir}/gideon
commondatadir = ${gideondatadir}/template-common

pythonhellodatadir = ${kde_datadir}/gideon/template-pythonhello
pythonhellodata_DATA = app.py app.kdevelop script

templatedatadir = ${gideondatadir}/templates
templatedata_DATA = pythonhello

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(pythonhellodatadir)/script


appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
pythonhellodir = ${appwizarddatadir}/template-pythonhello
templatedir = ${appwizarddatadir}/templates

pythonhello_DATA = app.py app.kdevelop script
template_DATA = pythonhello

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(pythonhellodir)/script


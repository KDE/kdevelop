appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
pythonhellodir = ${appwizarddatadir}/template-pythonhello
templatedir = ${appwizarddatadir}/templates

pythonhello_DATA = app.py app.kdevelop
template_DATA = pythonhello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(INSTALL_DATA) script.local $(DESTDIR)$(pythonhellodir)/script


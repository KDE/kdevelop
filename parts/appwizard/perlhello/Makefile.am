appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
perlhellodir = ${appwizarddatadir}/template-perlhello
templatedir = ${appwizarddatadir}/templates

perlhello_DATA = app.pl app.kdevelop camel.png
template_DATA = perlhello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(perlhellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(perlhellodir)/script

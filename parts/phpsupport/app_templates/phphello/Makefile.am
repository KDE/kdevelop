appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
phphellodir = ${appwizarddatadir}/template-phphello
templatedir = ${appwizarddatadir}/templates

phphello_DATA = app.php app.kdevelop
template_DATA = phphello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(phphellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(phphellodir)/script

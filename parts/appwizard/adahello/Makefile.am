appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
adahellodir = ${appwizarddatadir}/template-adahello
templatedir = ${appwizarddatadir}/templates

adahello_DATA = main.adb app.kdevelop
template_DATA = adahello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(adahellodir)
	$(INSTALL_DATA) ${srcdir}/script.local $(DESTDIR)$(adahellodir)/script

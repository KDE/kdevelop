appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
fpchellodir = ${appwizarddatadir}/template-fpchello
templatedir = ${appwizarddatadir}/templates

fpchello_DATA = main.pp app.kdevelop
template_DATA = fpchello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(fpchellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(fpchellodir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
pascalhellodir = ${appwizarddatadir}/template-pascalhello
templatedir = ${appwizarddatadir}/templates

pascalhello_DATA = main.pp app.kdevelop
template_DATA = pascalhello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(pascalhellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(pascalhellodir)/script

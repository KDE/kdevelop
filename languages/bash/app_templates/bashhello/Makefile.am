appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
bashhellodir = ${appwizarddatadir}/template-bashhello
templatedir = ${appwizarddatadir}/templates

bashhello_DATA = app.sh app.kdevelop
template_DATA = bashhello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(bashhellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(bashhellodir)/script

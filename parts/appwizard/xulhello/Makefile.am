appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
xulhellodir = ${appwizarddatadir}/template-xulhello
templatedir = ${appwizarddatadir}/templates

xulhello_DATA = app.xul app.kdevelop
template_DATA = xulhello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(xulhellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(xulhellodir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
haskellhellodir = ${appwizarddatadir}/template-haskellhello
templatedir = ${appwizarddatadir}/templates

haskellhello_DATA = Main.hs app.kdevelop haskellhello.png
template_DATA = haskellhello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(haskellhellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(haskellhellodir)/script


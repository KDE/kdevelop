appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
rubyhellodir = ${appwizarddatadir}/template-rubyhello
templatedir = ${appwizarddatadir}/templates

rubyhello_DATA = app.rb app.kdevelop
template_DATA = rubyhello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(rubyhellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(rubyhellodir)/script

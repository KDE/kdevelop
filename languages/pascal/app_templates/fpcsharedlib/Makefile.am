appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
fpcsharedlibdir = ${appwizarddatadir}/template-fpcsharedlib
templatedir = ${appwizarddatadir}/templates

fpcsharedlib_DATA = main.pp app.kdevelop
template_DATA = fpcsharedlib

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(fpcsharedlibdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(fpcsharedlibdir)/script

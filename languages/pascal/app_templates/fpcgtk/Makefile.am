appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
fpcgtkdir = ${appwizarddatadir}/template-fpcgtk
templatedir = ${appwizarddatadir}/templates

fpcgtk_DATA = main.pp app.kdevelop
template_DATA = fpcgtk

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(fpcgtkdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(fpcgtkdir)/script

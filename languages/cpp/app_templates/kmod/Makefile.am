appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kmoddir = ${appwizarddatadir}/template-kmod
templatedir = ${appwizarddatadir}/templates

kmod_DATA = src-Makefile kmod.kdevelop kmod-driver.c
template_DATA = kmod

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kmoddir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kmoddir)/script

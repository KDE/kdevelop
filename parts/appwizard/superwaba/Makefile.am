appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
superwabadir = ${appwizarddatadir}/template-superwaba
templatedir = ${appwizarddatadir}/templates

superwaba_DATA = superwaba.png sw.java src-Makefile sw.kdevelop sw.filelist
template_DATA = superwaba

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(INSTALL_DATA) script.local $(DESTDIR)$(superwabadir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kioslavedir = ${appwizarddatadir}/template-kioslave
templatedir = ${appwizarddatadir}/templates

kioslave_DATA = slave.cpp slave.h slave.protocol slave.kdevelop src-Makefile.am                 kioslave.png subdirs
template_DATA = kioslave

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kioslavedir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kioslavedir)/script

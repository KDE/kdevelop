appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kioslavedir = ${appwizarddatadir}/template-kioslave
templatedir = ${appwizarddatadir}/templates

kioslave_DATA = slave.cpp slave.h slave.protocol src-Makefile.am \
                kioslave.png subdirs
template_DATA = kioslave

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(kioslavedir)/script


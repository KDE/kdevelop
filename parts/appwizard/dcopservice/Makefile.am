appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
dcopservicedir = ${appwizarddatadir}/template-dcopservice
templatedir = ${appwizarddatadir}/templates

dcopservice_DATA = dcopservice.desktop   dcopservice.png \
	dcopservice.h  src-Makefile.am\
	dcopservice.cpp  dcopservice.kdevelop subdirs

template_DATA = dcopservice
DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(dcopservicedir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(dcopservicedir)/script
	rm script.local

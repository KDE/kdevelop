appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
qtopiasimpledir = ${appwizarddatadir}/template-qtopiasimple
templatedir = ${appwizarddatadir}/templates

qtopiasimple_DATA = main.cpp  \
		 qtopiasimple.png app.kdevelop app.pro src.pro
template_DATA = qtopiasimple

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(qtopiasimpledir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(qtopiasimpledir)/script
	rm script.local


appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
qmakesimpledir = ${appwizarddatadir}/template-qmakesimple
templatedir = ${appwizarddatadir}/templates

qmakesimple_DATA = main.cpp  \
		 qmakesimple.png app.kdevelop app.pro src.pro
template_DATA = qmakesimple

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(qmakesimple)
	$(INSTALL_DATA) script.local $(DESTDIR)$(qmakesimpledir)/script

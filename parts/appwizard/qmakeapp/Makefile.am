appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
qmakeappdir = ${appwizarddatadir}/template-qmakeapp
templatedir = ${appwizarddatadir}/templates

qmakeapp_DATA = main.cpp qmakeapp.cpp qmakeapp.h \
		   qmakeapp.png qmakeapp.kdevelop qmakeapp.pro src.pro \
                   filesave.xpm fileopen.xpm fileprint.xpm

template_DATA = qmakeapp

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(qmakeappdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(qmakeappdir)/script

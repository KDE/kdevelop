appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
qtopiaappdir = ${appwizarddatadir}/template-opieapp
templatedir = ${appwizarddatadir}/templates

qtopiaapp_DATA = Example.png  app.kdevelop  example.desktop   Makefile.am  app.pro       example.cpp      example.h        examplebase.ui  opieapp

template_DATA = opieapp

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(qtopiaappdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(qtopiaappdir)/script

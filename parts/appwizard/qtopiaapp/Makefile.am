appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
qtopiaappdir = ${appwizarddatadir}/template-qtopiaapp
templatedir = ${appwizarddatadir}/templates

qtopiaapp_DATA = Example.png  app.kdevelop  example.control  example.desktop  example.html    main.cpp 	Makefile.am  app.pro       example.cpp      example.h        examplebase.ui  qtopiaapp

template_DATA = qtopiaapp

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(qtopiaappdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(qtopiaappdir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
qtopiaappdir = ${appwizarddatadir}/template-opieinput
templatedir = ${appwizarddatadir}/templates

qtopiaapp_DATA = Makefile.am Example.png  app.kdevelop  app.pro       simpleimpl.cpp      simpleimpl.h      opieinput

template_DATA = opieinput

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(qtopiaappdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(qtopiaappdir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
qtopiaappdir = ${appwizarddatadir}/template-opietoday
templatedir = ${appwizarddatadir}/templates

qtopiaapp_DATA = Makefile.am Example.png  app.kdevelop  app.pro exampleplugin.cpp  examplepluginimpl.cpp  examplepluginwidget.cpp exampleplugin.h  examplepluginimpl.h  examplepluginwidget.h    opietoday

template_DATA = opietoday

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(qtopiaappdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(qtopiaappdir)/script

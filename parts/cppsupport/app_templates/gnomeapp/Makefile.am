appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
gnomeappdir = ${appwizarddatadir}/template-gnomeapp
templatedir = ${appwizarddatadir}/templates

gnomeapp_DATA = app.c app.h menus.c menus.h main.c 		src-Makefile.am configure.in gnomeapp.png app.kdevelop app.desktop
template_DATA = gnomeapp

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(gnomeappdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(gnomeappdir)/script

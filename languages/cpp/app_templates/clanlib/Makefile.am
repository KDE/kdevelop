# some directories
appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
clanlibdir = ${appwizarddatadir}/template-clanlib
templatedir = ${appwizarddatadir}/templates

clanlib_DATA = src-Makefile.am app-Makefile.am main.cpp app.kdevelop \
	app-configure.in app-autogen.sh
template_DATA = clanlib

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(clanlibdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(clanlibdir)/script

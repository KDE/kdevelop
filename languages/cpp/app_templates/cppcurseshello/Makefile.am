appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
cppcurseshellodir = ${appwizarddatadir}/template-cppcurseshello
templatedir = ${appwizarddatadir}/templates

cppcurseshello_DATA = main.cpp src-Makefile.am cpp-Makefile.am cpp-Makefile.cvs app-configure.in app.kdevelop
template_DATA = cppcurseshello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(cppcurseshellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(cppcurseshellodir)/script

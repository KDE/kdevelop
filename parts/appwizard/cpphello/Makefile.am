appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
cpphellodir = ${appwizarddatadir}/template-cpphello
templatedir = ${appwizarddatadir}/templates

cpphello_DATA = main.cpp src-Makefile.am cpp-Makefile.am cpp-Makefile.cvs 		configure.in cpphello.png app.kdevelop
template_DATA = cpphello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(cpphellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(cpphellodir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
cppsdlhellodir = ${appwizarddatadir}/template-cppsdlhello
templatedir = ${appwizarddatadir}/templates

cppsdlhello_DATA = main.cpp src-Makefile.am cpp-Makefile.am cpp-Makefile.cvs 		configure.in cppsdlhello.png app.kdevelop acinclude.m4
template_DATA = cppsdlhello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(cppsdlhellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(cppsdlhellodir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
fortranhellodir = ${appwizarddatadir}/template-fortranhello
templatedir = ${appwizarddatadir}/templates

fortranhello_DATA = main.f77 src-Makefile.am fortran-Makefile.am fortran-Makefile.cvs \
		configure.in fortranhello.png app.kdevelop
template_DATA = fortranhello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(INSTALL_DATA) script.local $(DESTDIR)$(fortranhellodir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
cpphellodir = ${appwizarddatadir}/template-cpphello
templatedir = ${appwizarddatadir}/templates

cpphello_DATA = main.cpp src-Makefile.am cpp-Makefile.am cpp-Makefile.cvs \
		configure.in cpphello.png app.kdevelop
template_DATA = cpphello

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(cpphellodir)/script


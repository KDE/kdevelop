appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
chellodir = ${appwizarddatadir}/template-chello
templatedir = ${appwizarddatadir}/templates

chello_DATA = main.c src-Makefile.am c-Makefile.am c-Makefile.cvs \
	      configure.in chello.png app.kdevelop
template_DATA = chello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(INSTALL_DATA) script.local $(DESTDIR)$(chellodir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
chellogbadir = ${appwizarddatadir}/template-chellogba
templatedir = ${appwizarddatadir}/templates

chellogba_DATA = main.c font.raw.c master.pal.c src-Makefile.am c-Makefile.am c-Makefile.cvs \
	      configure.in chellogba.png app.kdevelop main.h gba.h font.bmp README.devel
template_DATA = chellogba

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(INSTALL_DATA) script.local $(DESTDIR)$(chellogbadir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kickerdir = ${appwizarddatadir}/template-kicker
templatedir = ${appwizarddatadir}/templates

kicker_DATA = applet.cpp applet.h applet.desktop applet.kdevelop src-Makefile.am \
	      kicker.png subdirs
template_DATA = kicker

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kickerdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kickerdir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kickerdir = ${appwizarddatadir}/template-kicker
templatedir = ${appwizarddatadir}/templates

kicker_DATA = applet.cpp applet.h applet.desktop src-Makefile.am \
	      kicker.png subdirs
template_DATA = kicker

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(kickerdir)/script


appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kscreensaverdir = ${appwizarddatadir}/template-kscreensaver
templatedir = ${appwizarddatadir}/templates

kscreensaver_DATA = kscreensaver.desktop   kscreensaver.png \
	kscreensaver.h kscreensaverui.ui  src-Makefile.am\
	kscreensaver.cpp  kscreensaver.kdevelop subdirs

template_DATA = kscreensaver
DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kscreensaverdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kscreensaverdir)/script

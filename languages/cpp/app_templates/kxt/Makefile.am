# some directories
appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kxtdir = ${appwizarddatadir}/template-kxt
templatedir = ${appwizarddatadir}/templates

# all kxt data files
kxt_DATA = src-Makefile.am app.cpp app.h main.cpp appui.rc app.kdevelop \
	subdirs app.desktop preview.png appview.h appview.cpp appview_base.ui \
	prefs-base.ui prefs.cpp prefs.h app.kcfg settings.kcfgc README src-configure.in.in
template_DATA = kxt

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kxtdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kxtdir)/script

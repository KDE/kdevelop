# some directories
appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
khellodir = ${appwizarddatadir}/template-khello2
templatedir = ${appwizarddatadir}/templates

# all khello2 data files
khello_DATA = src-Makefile.am app.cpp app.h main.cpp appui.rc app.kdevelop \
	subdirs app.desktop preview.png widget.cpp widget.h widgetbase.ui
template_DATA = khello2

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(khellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(khellodir)/script

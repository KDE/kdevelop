appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
wxhellodir = ${appwizarddatadir}/template-wxhello
templatedir = ${appwizarddatadir}/templates

wxhello_DATA = src-Makefile.am app.cpp app.h app.kdevelop subdirs app.desktop wxhello.png
template_DATA = wxhello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(wxhellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(wxhellodir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
khellodir = ${appwizarddatadir}/template-khello
templatedir = ${appwizarddatadir}/templates

khello_DATA = src-Makefile.am app.cpp app.h main.cpp \
              appui.rc app.kdevelop subdirs app.desktop
template_DATA = khello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(khellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(khellodir)/script
	rm script.local

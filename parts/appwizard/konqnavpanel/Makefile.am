appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
konqnavpaneldir = ${appwizarddatadir}/template-konqnavpanel
templatedir = ${appwizarddatadir}/templates

konqnavpanel_DATA = konqnavpanel.cpp konqnavpanel.h konqnavpanel.desktop konqnavpanel.kdevelop src-Makefile.am \
	      konqnavpanel.png subdirs
template_DATA = konqnavpanel

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(konqnavpaneldir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(konqnavpaneldir)/script

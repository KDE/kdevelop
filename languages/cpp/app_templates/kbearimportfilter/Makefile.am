appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kbearimportfilterdir = ${appwizarddatadir}/template-kbearimportfilter
templatedir = ${appwizarddatadir}/templates

kbearimportfilter_DATA = kbearimportfilter-configure.in.in plugin.cpp plugin.kdevelop src-Makefile.am kbearimportfilter kbearimportfilter.desktop plugin.h script subdirs preview.png
template_DATA = kbearimportfilter

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kbearimportfilterdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kbearimportfilterdir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kopartdir = ${appwizarddatadir}/template-kopart
templatedir = ${appwizarddatadir}/templates

kopart_DATA = kopart.kdevelop kopart_aboutdata.h kopart_part.cpp kopart_view.h src-Makefile.am \
	kopart.png kopart_factory.cpp kopart_part.h subdirs kopart.desktop kopart.rc \
	kopart_factory.h kopart_view.cpp x-kopart.desktop kde-configure.in.in main.cpp

template_DATA = kopart

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kopartdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kopartdir)/script
	rm script.local

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kdevpartdir = ${appwizarddatadir}/template-kdevpart
templatedir = ${appwizarddatadir}/templates

kdevpart_DATA = src-Makefile.am kdevpart_part.cpp kdevpart_part.h 		kdevpart_widget.h kdevpart_widget.cpp 		kdevpart_part.rc kdevpart.desktop app.kdevelop
template_DATA = kdevpart

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kdevpartdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kdevpartdir)/script

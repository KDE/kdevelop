appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kdevpart2dir = ${appwizarddatadir}/template-kdevpart2
templatedir = ${appwizarddatadir}/templates

kdevpart2_DATA = src-Makefile.am kdevpart_part.cpp kdevpart_part.h 		kdevpart_widget.h kdevpart_widget.cpp 		kdevpart_part.rc kdevpart.desktop app.kdevelop kdevpart-configure.in.in
template_DATA = kdevpart2

noinst_SCRIPTS = script.local

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kdevpart2dir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kdevpart2dir)/script

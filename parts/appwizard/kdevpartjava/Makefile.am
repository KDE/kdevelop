appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kdevpartjavadir = ${appwizarddatadir}/template-kdevpartjava
templatedir = ${appwizarddatadir}/templates

kdevpartjava_DATA = src-Makefile.am kdevpartjava_part.java \
	kdevpartjava_factory.h kdevpartjava_factory.cpp kdevpartjava_factory.java kdevpartjava_widget.java \
	kdevpartjava_part.rc kdevpartjava.desktop app.kdevelop
template_DATA = kdevpartjava

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kdevpartjavadir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kdevpartjavadir)/script
	rm script.local

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kdevpartjavadir = ${appwizarddatadir}/template-kdevpartjava
templatedir = ${appwizarddatadir}/templates

kdevpartjava_DATA = src-Makefile.am kdevpartjava_part.java \
	kdevpartjava_factory.h kdevpartjava_factory.cpp kdevpartjava_factory.java kdevpartjava_widget.java \
	kdevpartjava_part.rc kdevpartjava.desktop

template_DATA = kdevpartjava

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) ${srcdir}/script.local $(kdevpartjavadir)/script

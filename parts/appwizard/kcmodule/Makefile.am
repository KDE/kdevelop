appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kcmoduledir = ${appwizarddatadir}/template-kcmodule
templatedir = ${appwizarddatadir}/templates

kcmodule_DATA = module.cpp module.h module.desktop src-Makefile.am \
                kcmodule.png subdirs
template_DATA = kcmodule

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(kcmoduledir)/script


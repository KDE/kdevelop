appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kcmoduledir = ${appwizarddatadir}/template-kcmodule
templatedir = ${appwizarddatadir}/templates

kcmodule_DATA = module.cpp module.h module.desktop module.kdevelop src-Makefile.am \
                kcmodule.png subdirs
template_DATA = kcmodule

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kcmoduledir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kcmoduledir)/script

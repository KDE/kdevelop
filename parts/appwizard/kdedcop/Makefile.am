appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kdedcopdir = ${appwizarddatadir}/template-kdedcop
templatedir = ${appwizarddatadir}/templates

kdedcop_DATA = app.cpp app.h app_iface.h app.kdevelop main.cpp mainclass.cpp mainclass.h          kdedcop.png src-Makefile.am subdirs
template_DATA = kdedcop

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kdedcopdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kdedcopdir)/script


appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
cppgenerichellodir = ${appwizarddatadir}/template-cppgenerichello
templatedir = ${appwizarddatadir}/templates

cppgenerichello_DATA = main.cpp cpphello.png app.kdevelop app.prj
template_DATA = cppgenerichello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(cppgenerichellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(cppgenerichellodir)/script

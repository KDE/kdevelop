appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
javahellodir = ${appwizarddatadir}/template-javahello
templatedir = ${appwizarddatadir}/templates

javahello_DATA = Main.java build.xml javahello.filelist javahello.kdevelop
template_DATA = javahello

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(javahellodir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(javahellodir)/script
	rm script.local

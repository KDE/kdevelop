appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
noatunplugindir = ${appwizarddatadir}/template-noatunvisual
templatedir = ${appwizarddatadir}/templates

noatunplugin_DATA = src-Makefile.am plugin_app.cpp plugin_app.h plugin_impl.cpp plugin_impl.h noatunplugin.png plugin.plugin plugin.kdevelop subdirs noatunvisual-configure.in.in
template_DATA = noatunvisual

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(noatunplugindir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(noatunplugindir)/script

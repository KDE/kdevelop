gideondatadir = ${kde_datadir}/gideon
commondatadir = ${gideondatadir}/template-common

kpartplugindatadir = ${gideondatadir}/template-kpartplugin
kpartplugindata_DATA = src-Makefile.am plugin_app.cpp plugin_app.h \
		plugin_app.rc hi16-action-plugin.png hi22-action-plugin.png

templatedatadir = ${gideondatadir}/templates
templatedata_DATA = kpartplugin

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) $(srcdir)/script.local $(kpartplugindatadir)/script

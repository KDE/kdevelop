appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kpartappdir = ${appwizarddatadir}/template-kpartapp
templatedir = ${appwizarddatadir}/templates

kpartapp_DATA = src-Makefile.am app.cpp app.h app_part.cpp app_part.h main.cpp \
                app_part.rc app_shell.rc kpartapp.png app.kdevelop
template_DATA = kpartapp

BUILT_SOURCES = script.local

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local:
	$(INSTALL_DATA) script.local $(kpartappdir)/script

appwizarddatadir = ${kde_datadir}/kdevappwizard
commondatadir = ${appwizarddatadir}/template-common
kpartappdir = ${appwizarddatadir}/template-kpartapp
templatedir = ${appwizarddatadir}/templates

kpartapp_DATA = src-Makefile.am app.cpp app.h app_part.cpp app_part.h main.cpp \
                app_part.rc app_shell.rc kpartapp.png app.kdevelop subdirs
template_DATA = kpartapp

DISTCLEANFILES = script.local
EXTRA_DIST = script

perl = perl

script.local: ${srcdir}/script
	cp ${srcdir}/script script.local ; \
	perl -npi -e 's%^#\!.*$$%#!'${perl}' -I'${commondatadir}'%g;' script.local

install-data-local: script.local
	$(mkinstalldirs) $(DESTDIR)$(kpartappdir)
	$(INSTALL_DATA) script.local $(DESTDIR)$(kpartappdir)/script

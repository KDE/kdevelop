dataFiles = app.kcfg app.kdevelop appkmdi.cpp appkmdi.h appkmdiView.cpp \
appkmdiView.h INSTALL kmdikonsole.cpp kmdikonsole.h main.cpp \
messages.sh prefs.ui QUICKSTART README SConscript-doc SConscript-po SConscript-src \
SConstruct settings.kcfgc src-Makefile tips VERSION \
app_part.cpp app_part.h app_part.rc app_shell.rc

templateName= kscons_kmdi

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles} ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP) -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz

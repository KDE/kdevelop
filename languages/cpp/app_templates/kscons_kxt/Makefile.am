dataFiles = app.cpp app.h main.cpp appui.rc app.kdevelop \
appview.h appview.cpp appview_base.ui prefs-base.ui \
prefs.cpp prefs.h app.kcfg settings.kcfgc messages.sh \
src-Makefile SConstruct SConscript-doc SConscript-po SConscript-src tips VERSION \
QUICKSTART INSTALL README

templateName= kscons_kxt

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

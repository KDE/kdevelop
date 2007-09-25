dataFiles = kapp4.cpp kapp4.h prefs_base.ui kapp4view.cpp kapp4view.h kapp4view_base.ui \
	main.cpp kde4appui.rc CMakeLists.txt kapp4.desktop kapp4.kcfg settings.kcfgc \
	kde4app.png README kde4app.kdevelop kde4app.kdevelop.filelist

templateName = kde4app

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP_COMMAND) -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz

dataFiles = main.cpp cmakeqt3app.cpp cmakeqt3app.h cmakeqt3app.png \
	filesave.xpm fileopen.xpm fileprint.xpm CMakeLists.txt CMakeLists.txt.src

templateName = cmakeqt3app

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP) -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz

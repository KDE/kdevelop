# all khello data files
dataFiles = src-Makefile.am app.cpp app.h main.cpp appui.rc app.kdevelop \
	subdirs app.desktop khello.png
templateName = khello

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz:
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP) -9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz

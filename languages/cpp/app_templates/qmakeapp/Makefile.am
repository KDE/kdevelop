dataFiles = main.cpp qmakeapp.cpp qmakeapp.h qmakeapp.png qmakeapp.kdevelop \
	qmakeapp.pro src.pro filesave.xpm fileopen.xpm fileprint.xpm

templateName = qmakeapp

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
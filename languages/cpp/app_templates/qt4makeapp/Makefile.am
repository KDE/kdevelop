dataFiles = main.cpp qt4makeapp.cpp qt4makeapp.h qt4makeapp.png qt4makeapp.kdevelop \
	qt4makeapp.pro src.pro application.qrc ReadMe filesave.xpm fileopen.xpm filenew.xpm \
	editcopy.xpm editpaste.xpm editcut.xpm

templateName = qt4makeapp

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
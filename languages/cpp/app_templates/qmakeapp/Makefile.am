dataFiles = main.cpp qmakeapp.cpp qmakeapp.h qmakeapp.png qmakeapp.kdevelop \
	qmakeapp.pro src.pro filesave.xpm fileopen.xpm fileprint.xpm

template_DATA = qmakeapp

### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
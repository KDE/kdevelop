dataFiles = dcopservice.desktop dcopservice.png dcopservice.h \
	src-Makefile.am main.cpp dcopservice.cpp dcopservice.kdevelop \
	subdirs
template_DATA = dcopservice
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
dataFiles = prc-tool.png src-Makefile prc-tool.kdevelop callback.h \
	palmhello.c palmhello.pbitm palmhello.rcp palmhelloRsc.h
template_DATA = prc-tool
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
dataFiles = konqnavpanel.cpp konqnavpanel.h konqnavpanel.desktop \
	konqnavpanel.kdevelop src-Makefile.am \
	konqnavpanel.png subdirs
template_DATA = konqnavpanel
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
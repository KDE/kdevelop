dataFiles = kbearimportfilter-configure.in.in plugin.cpp plugin.kdevelop \
	src-Makefile.am kbearimportfilter kbearimportfilter.desktop \
	plugin.h script subdirs preview.png
template_DATA = kbearimportfilter
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
dataFiles = kbearimportfilter-configure.in.in plugin.cpp plugin.kdevelop \
	src-Makefile.am kbearimportfilter kbearimportfilter.desktop \
	plugin.h script subdirs preview.png
templateName = kbearimportfilter

### no need to change below:
template_DATA = ${templateName}.kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
${templateName}.tar.gz:
	$(TAR) -cf ${templateName}.tar -C $(srcdir) ${dataFiles}
	$(GZIP) -9 ${templateName}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${templateName}.tar.gz ${templateName}.png

CLEANFILES = *.tar.gz
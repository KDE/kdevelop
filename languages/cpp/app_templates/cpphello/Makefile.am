dataFiles = main.cpp src-Makefile.am cpp-Makefile.am cpp-Makefile.cvs \
	configure.in cpphello.png app.kdevelop
templateName = cpphello

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
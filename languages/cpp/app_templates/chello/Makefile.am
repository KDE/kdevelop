dataFiles = main.c src-Makefile.am c-Makefile.am c-Makefile.cvs \
	configure.in chello.png app.kdevelop

templateName = chello

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
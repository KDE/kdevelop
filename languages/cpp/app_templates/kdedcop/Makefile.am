dataFiles = app.cpp app.h app_iface.h app.kdevelop main.cpp mainclass.cpp \
	mainclass.h kdedcop.png src-Makefile.am subdirs

templateName = kdedcop

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
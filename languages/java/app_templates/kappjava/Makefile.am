dataFiles = app.java pref.java appview.java  app_client.java appui.rc \
	src-Makefile.am kappjava.png app.kdevelop subdirs
templateName = kappjava

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
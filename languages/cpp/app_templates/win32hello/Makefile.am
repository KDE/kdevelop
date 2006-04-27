dataFiles = app.cpp src-Makefile.am win32-Makefile.am win32-Makefile.cvs \
	win32-configure.in win32hello.png app.kdevelop

templateName = win32hello

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP_COMMAND) -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz

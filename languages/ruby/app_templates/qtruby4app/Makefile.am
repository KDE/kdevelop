dataFiles =  main.rb qtruby4app.rb qtruby4app.png qtruby4app.kdevelop \
	application.qrc ReadMe filesave.xpm fileopen.xpm filenew.xpm \
	editcopy.xpm editpaste.xpm editcut.xpm app-Makefile app.filelist

templateName = qtruby4app

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


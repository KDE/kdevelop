dataFiles = main.adb app.kdevelop ada-Makefile app.kdevelop.filelist

### no need to change below:
template_DATA = adahello.kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
adahello.tar.gz:
	$(TAR) -cf adahello.tar -C $(srcdir) ${dataFiles}
	$(GZIP_COMMAND) -f9 adahello.tar

archivedir = ${appwizarddatadir}
archive_DATA = adahello.tar.gz adahello.png

CLEANFILES = *.tar.gz


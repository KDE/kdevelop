appwizarddatadir = ${kde_datadir}/kdevappwizard

dataFiles = main.adb app.kdevelop ada-Makefile app.kdevelop.filelistp

templatedir = ${appwizarddatadir}/templates
template_DATA = adahello

chello.tar.gz:
	$(TAR) -cf adahello.tar -C $(srcdir) ${dataFiles}
	gzip adahello.tar

archivedir = ${appwizarddatadir}
archive_DATA = adahello.tar.gz

CLEANFILES = *.tar.gz
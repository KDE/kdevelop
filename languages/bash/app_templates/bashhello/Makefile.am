appwizarddatadir = ${kde_datadir}/kdevappwizard

dataFiles = app.sh app.kdevelop

templatedir = ${appwizarddatadir}/templates
template_DATA = bashhello

chello.tar.gz:
	$(TAR) -cf chello.tar -C $(srcdir) ${dataFiles}
	gzip bashhello.tar

archivedir = ${appwizarddatadir}
archive_DATA = bashhello.tar.gz

CLEANFILES = *.tar.gz
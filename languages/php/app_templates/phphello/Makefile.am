appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

dataFiles = app.php app.kdevelop
template_DATA = phphello

phphello.tar.gz:
	$(TAR) -cf phphello.tar -C ${srcdir} ${dataFiles}
	gzip phphello.tar

archivedir = ${appwizarddatadir}
archive_DATA = phphello.tar.gz

CLEANFILES = *.tar.gz


appwizarddatadir = ${kde_datadir}/kdevappwizard

dataFiles = main.c src-Makefile.am c-Makefile.am c-Makefile.cvs \
	configure.in chello.png app.kdevelop

templatedir = ${appwizarddatadir}/templates
template_DATA = chello

chello.tar.gz:
	$(TAR) -cf chello.tar -C $(srcdir) ${dataFiles}
	gzip chello.tar

archivedir = ${appwizarddatadir}
archive_DATA = chello.tar.gz

CLEANFILES = *.tar.gz
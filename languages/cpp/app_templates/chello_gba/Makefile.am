dataFiles = main.c font.raw.c master.pal.c src-Makefile.am c-Makefile.am \
	c-Makefile.cvs configure.in chellogba.png app.kdevelop \
	main.h gba.h font.bmp README.devel
template_DATA = chellogba
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
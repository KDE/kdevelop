dataFiles = main.c font.raw.c master.pal.c src-Makefile.am c-Makefile.am \
	c-Makefile.cvs configure.in chellogba.png app.kdevelop \
	main.h gba.h font.bmp README.devel
templateName = chellogba

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles} ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	gzip -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz

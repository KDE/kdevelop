dataFiles = app.cpp app.h app_iface.h app.kdevelop main.cpp mainclass.cpp \
	mainclass.h kdedcop.png src-Makefile.am subdirs

template_DATA = kdedcop
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
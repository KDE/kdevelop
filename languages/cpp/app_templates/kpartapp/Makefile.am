dataFiles = src-Makefile.am app.cpp app.h app_part.cpp\
	app_part.h main.cpp app_part.rc app_shell.rc \
	kpartapp.png app.kdevelop subdirs
template_DATA = kpartapp
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
dataFiles = src-Makefile.am kdevpart_part.cpp kdevpart_part.h\
	kdevpart_widget.h kdevpart_widget.cpp \
	kdevpart_part.rc kdevpart.desktop app.kdevelop \
	kdevpart-configure.in.in README.dox
template_DATA = kdevpart2

### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
dataFiles = src-Makefile.am kdevlang_part.cpp kdevlang_part.h kdevlang.rc \
	kdevlang.desktop app.kdevelop kdevlang-configure.in.in README.dox
template_DATA = kdevlang
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
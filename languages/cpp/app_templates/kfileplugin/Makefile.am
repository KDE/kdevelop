dataFiles = README.devel kfile_plugin.cpp kfile_plugin.desktop \
	kfile_plugin.h plugin.kdevelop kfileplugin preview.png \
	src-Makefile.am subdirs
template_DATA = kfileplugin
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
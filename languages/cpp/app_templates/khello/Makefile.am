# all khello data files
dataFiles = src-Makefile.am app.cpp app.h main.cpp appui.rc app.kdevelop \
	subdirs app.desktop preview.png
template_DATA = khello
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
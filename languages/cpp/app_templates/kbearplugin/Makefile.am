dataFiles = kbearplugin.desktop subdirs hi32-action-plugin.png \
	outputwidget.cpp plugin.h script configwidget.cpp \
	outputwidget.h plugin.rc src-Makefile.am Makefile.am \
	kbearplugin-configure.in.in configwidget.h plugin.cpp \
	plugin.kdevelop preview.png
template_DATA = kbearplugin
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
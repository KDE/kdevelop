template_DATA = kateplugin
dataFiles = src-Makefile.am plugin_app.cpp plugin_app.h\
		plugin_app.rc hi16-action-plugin.png \
		hi22-action-plugin.png kateplugin.png \
		plugin.kdevelop subdirs plugin.desktop
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
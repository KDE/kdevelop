dataFiles = src-Makefile.am plugin_app.cpp plugin_app.h  \
	plugin_app.rc hi16-action-plugin.png hi22-action-plugin.png \
	kpartplugin.png plugin.kdevelop subdirs
	
templateName = kpartplugin

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP_COMMAND) -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz
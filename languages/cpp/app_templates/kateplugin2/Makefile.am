dataFiles = src-Makefile.am plugin_app.cpp plugin_app.h \
	plugin_app.rc hi16-action-plugin.png hi22-action-plugin.png \
	kateplugin2.png plugin.kdevelop subdirs plugin.desktop

templateName = kateplugin2

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
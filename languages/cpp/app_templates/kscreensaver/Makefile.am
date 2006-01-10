dataFiles = app.desktop kscreensaver.png app.h \
	appui.ui src-Makefile.am app.cpp \
	app.kdevelop subdirs

templateName= kscreensaver

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	gzip -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz $(templateName).png

CLEANFILES = *.tar.gz
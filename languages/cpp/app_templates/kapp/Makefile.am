dataFiles = app.cpp app.h pref.cpp pref.h appview.cpp appview.h \
	appiface.h app_client.cpp main.cpp appui.rc src-Makefile.am \
	kapp.png app.kdevelop subdirs README

templateName = kapp

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
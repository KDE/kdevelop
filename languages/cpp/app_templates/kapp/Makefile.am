dataFiles = app.cpp app.h pref.cpp pref.h appview.cpp appview.h \
	appiface.h app_client.cpp main.cpp appui.rc src-Makefile.am \
	kapp.png app.kdevelop subdirs README

template_DATA = kapp

### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
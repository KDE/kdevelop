dataFiles = src-Makefile.am app.cpp app.h main.cpp appui.rc app.kdevelop \
	subdirs app.desktop preview.png appview.h appview.cpp appview_base.ui \
	prefs-base.ui prefs.cpp prefs.h app.kcfg settings.kcfgc README \
	src-configure.in.in
template_DATA = kxt

### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
dataFiles = kscreensaver.desktop kscreensaver.png kscreensaver.h \
	kscreensaverui.ui src-Makefile.am kscreensaver.cpp \
	kscreensaver.kdevelop subdirs

template_DATA = kscreensaver

### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
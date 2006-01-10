dataFiles = kscreensaver.desktop kscreensaver.png kscreensaver.h \
	kscreensaverui.ui src-Makefile.am kscreensaver.cpp \
	kscreensaver.kdevelop subdirs

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
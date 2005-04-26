dataFiles = src-Makefile.am app-Makefile.am main.cpp app.kdevelop \
	app-configure.in app-autogen.sh
templateName = clanlib

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP) -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz $(templateName).png

CLEANFILES = *.tar.gz

dataFiles = main.cpp cpphello.png app.kdevelop app.prj
templateName = cppgenerichello

# BELOW is not the GENERIC Template-Template;
# png-filename differs from archive file name

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	gzip -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz cpphello.png

CLEANFILES = *.tar.gz
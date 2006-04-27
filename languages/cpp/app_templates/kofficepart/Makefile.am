dataFiles = kopart.kdevelop kopart_aboutdata.h kopart_part.cpp kopart_view.h \
	src-Makefile.am kopart.png kopart_factory.cpp kopart_part.h \
	subdirs kopart.desktop kopart.rc kopart_factory.h \
	kopart_view.cpp x-kopart.desktop kde-configure.in.in \
	main.cpp

templateName = kopart

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
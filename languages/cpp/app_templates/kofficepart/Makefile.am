dataFiles = kopart.kdevelop kopart_aboutdata.h kopart_part.cpp kopart_view.h \
	src-Makefile.am kopart.png kopart_factory.cpp kopart_part.h \
	subdirs kopart.desktop kopart.rc kopart_factory.h \
	kopart_view.cpp x-kopart.desktop kde-configure.in.in \
	main.cpp

template_DATA = kopart
#### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
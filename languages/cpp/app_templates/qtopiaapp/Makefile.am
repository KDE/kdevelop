dataFiles = Example.png app.kdevelop example.control example.desktop \
	example.html main.cpp Makefile.am app.pro example.cpp \
	example.h examplebase.ui qtopiaapp

template_DATA = qtopiaapp
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
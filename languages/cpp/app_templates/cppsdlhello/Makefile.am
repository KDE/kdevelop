dataFiles = main.cpp src-Makefile.am cpp-Makefile.am cpp-Makefile.cvs 		configure.in cppsdlhello.png app.kdevelop acinclude.m4
template_DATA = cppsdlhello

### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
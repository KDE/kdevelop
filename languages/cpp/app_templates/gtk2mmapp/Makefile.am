dataFiles = acconfig.h autogen.sh gtk2mm.glade main_window.cc \
	main_window_glade.hh Makefile.am app-Makefile.am \
	configure.in gtk2mm.gladep main_window.hh \
	app.kdevelop gtk2mm.cc gtk2mmapp main_window_glade.cc\
	src-Makefile.am config.h gtk2mm.png

template_DATA = gtk2mmapp

### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
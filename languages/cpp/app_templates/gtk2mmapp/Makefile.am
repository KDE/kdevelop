dataFiles = acconfig.h autogen.sh gtk2mm.glade main_window.cc \
	main_window_glade.hh Makefile.am app-Makefile.am \
	configure.in gtk2mm.gladep main_window.hh \
	app.kdevelop gtk2mm.cc gtk2mmapp main_window_glade.cc\
	src-Makefile.am config.h gtk2mmapp.png

templateName = gtk2mmapp

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz: ${dataFiles}
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	gzip -f9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz

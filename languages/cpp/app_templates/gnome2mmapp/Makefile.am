dataFiles = acconfig.h autogen.sh gnome2mm.glade main_window.cc \
	main_window_glade.hh Makefile.am app-Makefile.am \
	configure.in gnome2mm.gladep main_window.hh \
	app.kdevelop gnome2mm.cc gnome2mmapp main_window_glade.cc\
	src-Makefile.am config.h gnome2mmapp.png

templateName = gnome2mmapp

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
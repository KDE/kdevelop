

#original Makefile.am contents follow:

#dataFiles = app.cpp src-Makefile.am win32gui-Makefile.am win32gui-Makefile.cvs \
#	win32gui-configure.in win32gui.png app.kdevelop
#
#templateName = win32gui
#
#### no need to change below:
#template_DATA = $(templateName).kdevtemplate
#templatedir = ${appwizarddatadir}/templates
#
#appwizarddatadir = ${kde_datadir}/kdevappwizard
#$(templateName).tar.gz:
#	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
#	$(GZIP) -9 $(templateName).tar
#
#archivedir = ${appwizarddatadir}
#archive_DATA = $(templateName).tar.gz ${templateName}.png
#
#CLEANFILES = *.tar.gz

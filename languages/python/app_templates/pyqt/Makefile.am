dataFiles = app.py app.kdevelop fileopen.pyxpm fileprint.pyxpm  \
	filesave.pyxpm app-Makefile.am app-autogen.sh app-configure.ac \
	documentdetails.ui missing install-sh mkinstalldirs \
	pyqt-am-preproc.py templates-ui-dialog templates-ui-mainwin \
	pyqt.png
templateName = pyqt

### no need to change below:
template_DATA = $(templateName).kdevtemplate
templatedir = ${appwizarddatadir}/templates

appwizarddatadir = ${kde_datadir}/kdevappwizard
$(templateName).tar.gz:
	$(TAR) -cf $(templateName).tar -C $(srcdir) ${dataFiles}
	$(GZIP) -9 $(templateName).tar

archivedir = ${appwizarddatadir}
archive_DATA = $(templateName).tar.gz ${templateName}.png

CLEANFILES = *.tar.gz
dataFiles = app.py app.kdevelop fileopen.pyxpm fileprint.pyxpm  \
	filesave.pyxpm app-Makefile.am app-autogen.sh app-configure.ac \
	documentdetails.ui missing install-sh mkinstalldirs \
	pyqt-am-preproc.py templates-ui-dialog templates-ui-mainwin \
	pyqt.png
template_DATA = pyqt
### no need to change below:
appwizarddatadir = ${kde_datadir}/kdevappwizard
templatedir = ${appwizarddatadir}/templates

${template_DATA}.tar.gz:
	$(TAR) -cf ${template_DATA}.tar -C $(srcdir) ${dataFiles}
	gzip ${template_DATA}.tar

archivedir = ${appwizarddatadir}
archive_DATA = ${template_DATA}.tar.gz

CLEANFILES = *.tar.gz
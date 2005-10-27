

#original Makefile.am contents follow:

#dataFiles = main.cpp qt4makeapp.cpp qt4makeapp.h qt4makeapp.png qt4makeapp.kdevelop \
#	qt4makeapp.pro src.pro filesave.png fileopen.png fileprint.png \
#	editcopy.png editpaste.png editcut.png
#
#templateName = qt4makeapp
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
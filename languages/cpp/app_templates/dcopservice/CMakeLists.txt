

#original Makefile.am contents follow:

#dataFiles = dcopservice.desktop dcopservice.png dcopservice.h \
#	src-Makefile.am main.cpp dcopservice.cpp dcopservice.kdevelop \
#	subdirs
#templateName = dcopservice
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
#######################
#previous CMakeLists.txt contents follow:




#original Makefile.am contents follow:

#dataFiles = README.devel kfile_plugin.cpp kfile_plugin.desktop \
#	kfile_plugin.h plugin.kdevelop kfileplugin kfileplugin.png \
#	src-Makefile.am subdirs
#templateName = kfileplugin
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


#original Makefile.am contents follow:

#dataFiles = test.cpp lib.h lib.cpp cmakelibcpp.png CMakeLists.txt CMakeLists.txt.src
#templateName = cmakelibcpp
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

#
#PROJECT(%{APPNAMELC})
#
##if you don't want the full compiler output, remove the following line
#SET(CMAKE_VERBOSE_MAKEFILE ON)
#
#
#SUBDIRS(src)
#
#

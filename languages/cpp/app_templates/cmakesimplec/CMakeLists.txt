

#original Makefile.am contents follow:

#dataFiles = main.c cmakesimplec.png CMakeLists.txt
#templateName = cmakesimplec
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
##add definitions, compiler switches, etc. 
#ADD_DEFINITIONS(-Wall -O2)
#
##list all source files here
#ADD_EXECUTABLE(%{APPNAMELC} main.c)
#
##need to link to some other libraries ? just add them here
##TARGET_LINK_LIBRARIES(%{APPNAMELC} png jpeg)
#

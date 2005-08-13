SUBDIRS( extensions external extras )

SET(kdevinterfaces_STAT_SRCS
kdevcore.cpp
kdevproject.cpp
kdevlanguagesupport.cpp
kdevpartcontroller.cpp
kdevapi.cpp
kdevmainwindow.cpp
kdevplugin.cpp
kdevcoderepository.cpp
codemodel.cpp
codemodel_treeparser.cpp
codemodel_utils.cpp
kdevdesignerintegration.cpp
kdevplugincontroller.cpp
kdevplugininfo.cpp
KDevCoreIface.cpp
KDevPartControllerIface.cpp
katedocumentmanagerinterface.cpp
kdevprojectiface.cpp
kdevdesignerintegrationiface.cpp
)

SET(kdevinterfaces_DCOP_SRCS
KDevCoreIface.h
KDevPartControllerIface.h
katedocumentmanagerinterface.h
kdevprojectiface.h
kdevdesignerintegrationiface.h
)

KDE_ADD_DCOP_SKELS(kdevinterfaces_STAT_SRCS ${kdevinterfaces_DCOP_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/external/ )

KDE_AUTOMOC(${kdevinterfaces_STAT_SRCS})

ADD_LIBRARY(kdevinterfaces STATIC ${kdevinterfaces_STAT_SRCS})

INSTALL_FILES(/share/servicetypes/ ".+\\.desktop")
INSTALL_FILES(/include/kdevelop/interfaces FILES
kdevlanguagesupport.h
kdevmainwindow.h
kdevpartcontroller.h
kdevplugin.h
kdevproject.h
kdevcore.h
kdevcoderepository.h
codemodel.h
codemodel_utils.h
codemodel_treeparser.h
kdevgenericfactory.h
kdevapi.h
kdevdesignerintegration.h
kdevplugincontroller.h
kdevplugininfo.h
KDevCoreIface.h
KDevPartControllerIface.h
katedocumentmanagerinterface.h )


#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	$(all_includes)
#
#noinst_LTLIBRARIES = libkdevinterfaces.la
#
#libkdevinterfaces_la_SOURCES = kdevcore.cpp kdevproject.cpp \
#		kdevlanguagesupport.cpp kdevpartcontroller.cpp kdevapi.cpp KDevCoreIface.skel \
#		kdevmainwindow.cpp KDevPartControllerIface.skel kdevplugin.cpp kdevcoderepository.cpp \
#		codemodel.cpp codemodel_treeparser.cpp codemodel_utils.cpp \
#		kdevdesignerintegration.cpp kdevplugincontroller.cpp kdevplugininfo.cpp KDevCoreIface.cpp \
#		KDevPartControllerIface.cpp katedocumentmanagerinterface.cpp katedocumentmanagerinterface.skel \
#	kdevprojectiface.cpp kdevprojectiface.skel kdevdesignerintegrationiface.cpp \
#	kdevdesignerintegrationiface.skel
#
#kdevelopincludedir = $(includedir)/kdevelop/interfaces
#kdevelopinclude_HEADERS = kdevlanguagesupport.h kdevmainwindow.h \
#	kdevpartcontroller.h kdevplugin.h kdevproject.h kdevcore.h kdevcoderepository.h codemodel.h \
#	codemodel_utils.h codemodel_treeparser.h kdevgenericfactory.h kdevapi.h \
#	kdevdesignerintegration.h kdevplugincontroller.h kdevplugininfo.h KDevCoreIface.h \
#	KDevPartControllerIface.h katedocumentmanagerinterface.h
#
#
#servicetypedir = $(kde_servicetypesdir)
#servicetype_DATA = kdevelopproject.desktop kdeveloplanguagesupport.desktop \
#	kdevelopplugin.desktop
#
#METASOURCES = AUTO
#
#SUBDIRS = extensions external extras
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevutil designer_integration kdevcatalog kdevprofileslib kdevextensions
#DOXYGEN_PROJECTNAME = KDevelop Interfaces Library
#DOXYGEN_DOCDIRPREFIX = kdev
#include ../../Doxyfile.am
#noinst_HEADERS = kdevprojectiface.h kdevdesignerintegrationiface.h

#######################
#previous CMakeLists.txt contents follow:

#SUBDIRS(extensions external)
#
#SET(kdevinterfaces_SOURCES
#kdevcore.cpp
#kdevproject.cpp
#kdevlanguagesupport.cpp
#kdevpartcontroller.cpp
#kdevapi.cpp
#kdevmainwindow.cpp
#kdevplugin.cpp
#kdevcoderepository.cpp
#codemodel.cpp
#codemodel_treeparser.cpp
#codemodel_utils.cpp
#kdevdesignerintegration.cpp
#kdevplugincontroller.cpp
#kdevplugininfo.cpp
#KDevCoreIface.cpp
#KDevPartControllerIface.cpp
#katedocumentmanagerinterface.cpp
#kdevprojectiface.cpp
#kdevdesignerintegrationiface.cpp
#)
#
#SET(kdevinterfaces_DCOP_SRCS
#KDevCoreIface.h
#KDevPartControllerIface.h
#katedocumentmanagerinterface.h
#kdevprojectiface.h
#kdevdesignerintegrationiface.h
#)
#
#KDE_ADD_DCOP_SKELS(kdevinterfaces_SOURCES ${kdevinterfaces_DCOP_SRCS})
#
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/external/ )

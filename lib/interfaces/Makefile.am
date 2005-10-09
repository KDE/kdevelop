INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libkdevinterfaces.la

libkdevinterfaces_la_SOURCES = \
	kdevapi.cpp		kdevlanguagesupport.cpp      kdevplugininfo.cpp \
	kdevappfrontend.cpp	kdevmainwindow.cpp	     kdevprojectbuilder.cpp \
	kdevcatalog.cpp		kdevmakefrontend.cpp	     kdevproject.cpp \
	kdevcodemodel.cpp	kdevpartcontroller.cpp	     kdevprojecteditor.cpp \
	kdevcoderepository.cpp	KDevPartControllerIface.cpp  kdevprojectiface.cpp \
	kdevcore.cpp		kdevpcsimporter.cpp	     kdevprojectimporter.cpp \
	KDevCoreIface.cpp	kdevplugincontroller.cpp     kdevprojectmodel.cpp \
	kdevdifffrontend.cpp	kdevplugin.cpp		     tag.cpp \
	domutil.cpp 		kdevitemmodel.cpp	     urlutil.cpp \
	KDevPartControllerIface.skel KDevCoreIface.skel kdevprojectiface.skel \
	filetemplate.cpp

libkdevinterfaces_la_LDFLAGS = -no-undefined $(DB3LDFLAGS) $(all_libraries)
libkdevinterfaces_la_LIBADD = -l$(DB3LIB) $(LIB_KDEUI) $(LIB_KHTML) \
	$(LIB_KPARTS) -lktexteditor

kdevelopincludedir = $(includedir)/kdevelop/interfaces
kdevelopinclude_HEADERS = \
	kdevpartcontroller.h \
	kdevapi.h			KDevPartControllerIface.h \
	kdevappfrontend.h		kdevpcsimporter.h \
	kdevcatalog.h			kdevplugincontroller.h \
	kdevcodemodel.h			kdevplugin.h \
	kdevcoderepository.h		kdevplugininfo.h \
	kdevcore.h			kdevprojectbuilder.h \
	KDevCoreIface.h			kdevprojecteditor.h \
	kdevcreatefile.h		kdevproject.h \
	kdevdifffrontend.h		domutil.h \
	kdevgenericfactory.h		kdevprojectimporter.h \
	kdevlanguagesupport.h		kdevprojectmodel.h \
	kdevmainwindow.h		tag.h \
	kdevmakefrontend.h		kdevitemmodel.h \
	urlutil.h			filetemplate.h

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = \
	kdevelopappfrontend.desktop	 kdeveloppcsimporter.desktop \
	kdevelopcreatefile.desktop	 kdevelopplugin.desktop \
	kdevelopdifffrontend.desktop	 kdevelopprojectbuilder.desktop \
	kdeveloplanguagesupport.desktop  kdevelopproject.desktop \
	kdevelopmakefrontend.desktop	 kdevelopprojectimporter.desktop \
	kdevelop_part.desktop		 kdevelop_rwpart.desktop

METASOURCES = AUTO

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts \
	kutils kdevutil kdevcatalog kdevprofileslib kdevextensions
DOXYGEN_PROJECTNAME = KDevelop Interfaces Library
DOXYGEN_DOCDIRPREFIX = kdev
include ../../Doxyfile.am
noinst_HEADERS = kdevprojectiface.h

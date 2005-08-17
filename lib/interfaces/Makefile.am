INCLUDES = -I$(top_srcdir)/lib/util \
	$(all_includes)

noinst_LTLIBRARIES = libkdevinterfaces.la

libkdevinterfaces_la_SOURCES = \
	katedocumentmanagerinterface.cpp  KDevPartControllerIface.skel \
	kdevapi.cpp			  kdevpcsimporter.cpp \
	kdevappfrontend.cpp		  kdevplugincontroller.cpp \
	kdevcatalog.cpp			  kdevplugin.cpp \
	kdevcodemodel.cpp		  kdevplugininfo.cpp \
	kdevcoderepository.cpp		  kdevprojectbuilder.cpp \
	kdevcore.cpp			  kdevproject.cpp \
	KDevCoreIface.skel		  kdevprojecteditor.cpp \
	kdevdifffrontend.cpp		  kdevprojectiface.cpp \
	kdevlanguagesupport.cpp		  kdevprojectimporter.cpp \
	kdevmainwindow.cpp		  kdevprojectmodel.cpp \
	kdevmakefrontend.cpp		  tag.cpp \
	kdevpartcontroller.cpp

libkdevinterfaces_la_CXXFLAGS = -DNO_KATE_DOCUMENTMANAGER
libkdevinterfaces_la_LDFLAGS = \
	$(DB3LDFLAGS) \
	-no-undefined $(all_libraries)
libkdevinterfaces_la_LIBADD = \
	$(top_builddir)/lib/util/libkdevutil.la \
	-l$(DB3LIB) $(LIB_KDEUI) $(LIB_KHTML) $(LIB_KPARTS) \
	-lktexteditor -lkscript

kdevelopincludedir = $(includedir)/kdevelop/interfaces
kdevelopinclude_HEADERS = \
	katedocumentmanagerinterface.h	kdevpartcontroller.h \
	kdevapi.h			KDevPartControllerIface.h \
	kdevappfrontend.h		kdevpcsimporter.h \
	kdevcatalog.h			kdevplugincontroller.h \
	kdevcodemodel.h			kdevplugin.h \
	kdevcoderepository.h		kdevplugininfo.h \
	kdevcore.h			kdevprojectbuilder.h \
	KDevCoreIface.h			kdevprojecteditor.h \
	kdevcreatefile.h		kdevproject.h \
	kdevdifffrontend.h		\
	kdevgenericfactory.h		kdevprojectimporter.h \
	kdevlanguagesupport.h		kdevprojectmodel.h \
	kdevmainwindow.h		tag.h \
	kdevmakefrontend.h 

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

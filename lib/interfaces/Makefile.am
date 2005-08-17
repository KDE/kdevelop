INCLUDES = -I$(top_srcdir)/lib/util \
	$(all_includes)

noinst_LTLIBRARIES = libkdevinterfaces.la

libkdevinterfaces_la_SOURCES = \
	katedocumentmanagerinterface.cpp  kdevpcsimporter.cpp \
	kdevapi.cpp			  kdevplugincontroller.cpp \
	kdevcodemodel.cpp		  kdevplugin.cpp \
	kdevcoderepository.cpp		  kdevplugininfo.cpp \
	kdevcore.cpp			  kdevprojectbuilder.cpp \
	KDevCoreIface.skel		  kdevproject.cpp \
	kdevlanguagesupport.cpp		  kdevprojecteditor.cpp \
	kdevmainwindow.cpp		  kdevprojectiface.cpp \
	kdevpartcontroller.cpp		  kdevprojectimporter.cpp \
	KDevPartControllerIface.skel	  kdevprojectmodel.cpp

libkdevinterfaces_la_CXXFLAGS = -DNO_KATE_DOCUMENTMANAGER
libkdevinterfaces_la_LDFLAGS = \
	$(DB3LDFLAGS) \
	-no-undefined $(all_libraries)
libkdevinterfaces_la_LIBADD = \
	$(top_builddir)/lib/util/libkdevutil.la \
	-l$(DB3LIB) $(LIB_KDEUI) $(LIB_KHTML) $(LIB_KPARTS) 
	-lktexteditor -lkscript

kdevelopincludedir = $(includedir)/kdevelop/interfaces
kdevelopinclude_HEADERS = \
	katedocumentmanagerinterface.h	kdevpcsimporter.h \
	kdevapi.h			kdevplugincontroller.h \
	kdevcodemodel.h			kdevplugin.h \
	kdevcoderepository.h		kdevplugininfo.h \
	kdevcore.h			kdevprojectbuilder.h \
	KDevCoreIface.h			kdevprojecteditor.h \
	kdevgenericfactory.h		kdevproject.h \
	kdevlanguagesupport.h		kdevprojectiface.h \
	kdevmainwindow.h		kdevprojectimporter.h \
	kdevpartcontroller.h		kdevprojectmodel.h \
	KDevPartControllerIface.h


servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = \
	kdeveloplanguagesupport.desktop  kdevelopprojectbuilder.desktop \
	kdevelop_part.desktop		 kdevelopproject.desktop \
	kdeveloppcsimporter.desktop	 kdevelopprojectimporter.desktop \
	kdevelopplugin.desktop		 kdevelop_rwpart.desktop

METASOURCES = AUTO

SUBDIRS = extensions extras

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts \
	kutils kdevutil kdevcatalog kdevprofileslib kdevextensions
DOXYGEN_PROJECTNAME = KDevelop Interfaces Library
DOXYGEN_DOCDIRPREFIX = kdev
include ../../Doxyfile.am
noinst_HEADERS = kdevprojectiface.h

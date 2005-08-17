INCLUDES = -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	$(all_includes)

noinst_LTLIBRARIES = libkdevinterfaces.la

libkdevinterfaces_la_SOURCES = kdevcore.cpp kdevproject.cpp \
		kdevlanguagesupport.cpp kdevpartcontroller.cpp kdevapi.cpp KDevCoreIface.skel \
		kdevmainwindow.cpp KDevPartControllerIface.skel kdevplugin.cpp kdevcoderepository.cpp \
		codemodel.cpp codemodel_treeparser.cpp codemodel_utils.cpp \
		kdevdesignerintegration.cpp kdevplugincontroller.cpp kdevplugininfo.cpp KDevCoreIface.cpp \
		KDevPartControllerIface.cpp \
		kdevprojectiface.cpp kdevprojectiface.skel kdevdesignerintegrationiface.cpp \
		kdevdesignerintegrationiface.skel
libkdevinterfaces_la_CXXFLAGS = -DNO_KATE_DOCUMENTMANAGER
libkdevinterfaces_la_LDFLAGS = -no-undefined $(all_libraries)
libkdevinterfaces_la_LIBADD = $(top_builddir)/lib/interfaces/external/libkinterfacedesigner.la \
	$(top_builddir)/lib/util/libkdevutil.la \
	$(LIB_KDEUI) $(LIB_KHTML) $(LIB_KPARTS) -lktexteditor -lkscript

kdevelopincludedir = $(includedir)/kdevelop/interfaces
kdevelopinclude_HEADERS = kdevlanguagesupport.h kdevmainwindow.h \
	kdevpartcontroller.h kdevplugin.h kdevproject.h kdevcore.h kdevcoderepository.h codemodel.h \
	codemodel_utils.h codemodel_treeparser.h kdevgenericfactory.h kdevapi.h \
	kdevdesignerintegration.h kdevplugincontroller.h kdevplugininfo.h KDevCoreIface.h \
	KDevPartControllerIface.h katedocumentmanagerinterface.h


servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kdevelopproject.desktop kdeveloplanguagesupport.desktop \
	kdevelopplugin.desktop

METASOURCES = AUTO

SUBDIRS = extensions external extras

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts \
	kutils kdevutil designer_integration kdevcatalog kdevprofileslib kdevextensions
DOXYGEN_PROJECTNAME = KDevelop Interfaces Library
DOXYGEN_DOCDIRPREFIX = kdev
include ../../Doxyfile.am
noinst_HEADERS = kdevprojectiface.h kdevdesignerintegrationiface.h

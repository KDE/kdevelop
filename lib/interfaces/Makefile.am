INCLUDES = -I$(top_srcdir)/lib/external_interfaces -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/widgets/propeditor $(all_includes)

noinst_LTLIBRARIES = libkdevinterfaces.la

libkdevinterfaces_la_SOURCES = kdevcore.cpp kdevproject.cpp \
	kdevlanguagesupport.cpp kdevmakefrontend.cpp kdevappfrontend.cpp kdevcompileroptions.cpp \
	kdevpartcontroller.cpp kdevapi.cpp kdevdifffrontend.cpp KDevMakeFrontendIface.skel \
	KDevMakeFrontendIface.cpp KDevAppFrontendIface.skel KDevAppFrontendIface.cpp KDevCoreIface.skel \
	KDevCoreIface.cpp kdevmainwindow.cpp KDevPartControllerIface.skel \
	KDevPartControllerIface.cpp kdevplugin.cpp kdevdebugger.cpp kdevversioncontrol.cpp \
	kdevcreatefile.cpp kde30x_markinterfaceextension.cpp kde30x_texthintinterface.cpp \
	kdevcoderepository.cpp codemodel.cpp kdevvcsfileinfoprovider.cpp codemodel_treeparser.cpp \
	kdevbuildsystem.cpp codemodel_utils.cpp kdevpcsimporter.cpp kdevdocumentationplugin.cpp \
	kdevlicense.cpp kdevfile.cpp kdevdesignerintegration.cpp

kdevelopincludedir = $(includedir)/kdevelop/interfaces
kdevelopinclude_HEADERS = KDevAppFrontendIface.h kdevdebugger.h KDevCoreIface.h \
	kdevdifffrontend.h KDevMakeFrontendIface.h kdevversioncontrol.h KDevPartControllerIface.h \
	kdevlanguagesupport.h kde30x_markinterfaceextension.h kdevmainwindow.h \
	kde30x_texthintinterface.h kdevmakefrontend.h kdevcreatefile.h kdevpartcontroller.h \
	kdevappfrontend.h kdevplugin.h kdevcompileroptions.h kdevproject.h kdevcore.h \
	kdevcoderepository.h codemodel.h codemodel_utils.h codemodel_treeparser.h kdevbuildsystem.h \
	kdevpcsimporter.h kdevgenericfactory.h kdevvcsfileinfoprovider.h kdevapi.h \
	kdevdocumentationplugin.h kdevlicense.h kdevfile.h kdevdesignerintegration.h


servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kdevelopcompileroptions.desktop     kdevelopmakefrontend.desktop kdevelopappfrontend.desktop     kdevelopproject.desktop kdeveloplanguagesupport.desktop     kdevelopplugin.desktop kdevelopdifffrontend.desktop     kdevelopcreatefile.desktop kdevelopversioncontrol.desktop     kdevelopsourceformatter.desktop kdeveloppcsimporter.desktop kdevelopbuildsystems.desktop kdevelopdocumentationplugins.desktop

EXTRA_DIST = newlanguage.dox.h

METASOURCES = AUTO




SUBDIRS = extensions

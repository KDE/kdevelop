INCLUDES = $(all_includes) -I$(top_srcdir)/lib/util

noinst_LTLIBRARIES = libkdevinterfaces.la

libkdevinterfaces_la_SOURCES = kdevcore.cpp kdevproject.cpp kdevlanguagesupport.cpp kdevmakefrontend.cpp kdevappfrontend.cpp kdevcompileroptions.cpp kdevpartcontroller.cpp kdevapi.cpp kdevdifffrontend.cpp KDevMakeFrontendIface.skel KDevMakeFrontendIface.cpp KDevAppFrontendIface.skel KDevAppFrontendIface.cpp KDevCoreIface.skel KDevCoreIface.cpp kdevmainwindow.cpp KDevPartControllerIface.skel KDevPartControllerIface.cpp kdevplugin.cpp kdevdebugger.cpp kdevversioncontrol.cpp kdevcreatefile.cpp kde30x_markinterfaceextension.cpp kde30x_texthintinterface.cpp kdevsourceformatter.cpp kdevcoderepository.cpp codemodel.cpp

kdevelopincludedir = $(includedir)/kdevelop
kdevelopinclude_HEADERS = KDevAppFrontendIface.h kdevdebugger.h     KDevCoreIface.h	 kdevdifffrontend.h     KDevMakeFrontendIface.h kdevversioncontrol.h     KDevPartControllerIface.h kdevlanguagesupport.h     kde30x_markinterfaceextension.h kdevmainwindow.h     kde30x_texthintinterface.h kdevmakefrontend.h     kdevcreatefile.h kdevpartcontroller.h     kdevappfrontend.h kdevplugin.h     kdevcompileroptions.h kdevproject.h     kdevcore.h kdevsourceformatter.h     kdevcoderepository.h codemodel.h

noinst_HEADERS = kdevapi.h

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kdevelopcompileroptions.desktop     kdevelopmakefrontend.desktop kdevelopappfrontend.desktop     kdevelopproject.desktop kdeveloplanguagesupport.desktop     kdevelopplugin.desktop kdevelopdifffrontend.desktop     kdevelopcreatefile.desktop kdevelopversioncontrol.desktop     kdevelopsourceformatter.desktop

EXTRA_DIST = newlanguage.dox.h

METASOURCES = AUTO


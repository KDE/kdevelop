INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libkdevinterfaces.la

libkdevinterfaces_la_SOURCES = kdevcore.cpp kdevproject.cpp 			       kdevlanguagesupport.cpp kdevversioncontrol.cpp 			       kdevmakefrontend.cpp kdevappfrontend.cpp kdevcompileroptions.cpp 			       kdevpartcontroller.cpp kdevapi.cpp kdevdifffrontend.cpp 			       KDevMakeFrontendIface.skel KDevMakeFrontendIface.cpp 			       KDevAppFrontendIface.skel KDevAppFrontendIface.cpp 			       KDevCoreIface.skel KDevCoreIface.cpp kdevtoplevel.cpp 			       KDevPartControllerIface.skel KDevPartControllerIface.cpp 			       kdevplugin.cpp kdevdebugger.cpp kdevglobalversioncontrol.cpp kdevglobalversioncontrol.h

kdevelopincludedir = $(includedir)/kdevelop
kdevelopinclude_HEADERS = kdevcore.h kdevproject.h \
			  kdevlanguagesupport.h kdevversioncontrol.h kdevdifffrontend.h \
			  kdevmakefrontend.h kdevappfrontend.h kdevcompileroptions.h \
			  KDevMakeFrontendIface.h KDevAppFrontendIface.h KDevCoreIface.h \
			  kdevplugin.h kdevdebugger.h

noinst_HEADERS = kdevapi.h

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kdevelopcompileroptions.desktop \
		   kdevelopmakefrontend.desktop kdevelopappfrontend.desktop \
		   kdevelopproject.desktop kdeveloplanguagesupport.desktop \
		   kdevelopplugin.desktop kdevelopdifffrontend.desktop

METASOURCES = AUTO

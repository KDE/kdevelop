INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libkdevinterfaces.la

libkdevinterfaces_la_SOURCES = kdevfactory.cpp kdevpart.cpp kdevcore.cpp kdevproject.cpp \
			       kdevlanguagesupport.cpp kdevversioncontrol.cpp \
			       kdevmakefrontend.cpp kdevappfrontend.cpp kdevcompileroptions.cpp \
			       kdevpartcontroller.cpp kdevapi.cpp\
			       KDevMakeFrontendIface.skel KDevMakeFrontendIface.cpp \
			       KDevAppFrontendIface.skel KDevAppFrontendIface.cpp \
			       KDevCoreIface.skel KDevCoreIface.cpp kdevtoplevel.cpp \
			       KDevPartControllerIface.skel KDevPartControllerIface.cpp \
			       kdevplugin.cpp

kdevelopincludedir = $(includedir)/kdevelop
kdevelopinclude_HEADERS = kdevfactory.h kdevpart.h kdevcore.h kdevproject.h \
			  kdevlanguagesupport.h kdevversioncontrol.h \
			  kdevmakefrontend.h kdevappfrontend.h kdevcompileroptions.h \
			  KDevMakeFrontendIface.h KDevAppFrontendIface.h KDevCoreIface.h \
			  kdevplugin.h

noinst_HEADERS = kdevapi.h

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kdeveloppart.desktop kdevelopcompileroptions.desktop \
		   kdevelopmakefrontend.desktop kdevelopappfrontend.desktop \
		   kdevelopproject.desktop kdeveloplanguagesupport.desktop

METASOURCES = AUTO

INCLUDES = $(all_includes)

noinst_LTLIBRARIES = libkdevgeneral.la

libkdevgeneral_la_SOURCES = kdevcomponent.cpp kdevversioncontrol.cpp \
			    kdevlanguagesupport.cpp\
			    kdevmakefrontend.cpp kdevappfrontend.cpp kdeveditormanager.cpp \
			    ctoolclass.cpp processview.cpp kdevactions.cpp kdevnodes.cpp \
			    KDevEditorManagerIface.skel KDevEditorManagerIface.cpp \
			    KDevMakeFrontendIface.skel KDevMakeFrontendIface.cpp \
			    kdevelopcorebase.cpp

kdevelopincludedir = $(includedir)/kdevelop
kdevelopinclude_HEADERS = kdevcomponent.h kdevversioncontrol.h \
			  kdevlanguagesupport.h \
			  kdevmakefrontend.h kdevappfrontend.h kdeveditormanager.h \
			  ctoolclass.h processview.h \
			  kdevviewhandler.h kdevactions.h kdevnodes.h \
			  KDevEditorManagerIface.h KDevMakeFrontendIface.h \
			  kdevelopcorebase.h

noinst_HEADERS = kdevapi.h

METASOURCES = AUTO

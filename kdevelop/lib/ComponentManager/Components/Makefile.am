INCLUDES= $(all_includes) -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/lib/ComponentManager
noinst_LTLIBRARIES = libKDevComponentManagerComponents.la

libKDevComponentManagerComponents_la_SOURCES = kdevappfrontend.cpp kdevmakefrontend.cpp kdeveditormanager.cpp kdevlanguagesupport.cpp \
			kdevversioncontrol.cpp KDevClassStore.cpp

noinst_HEADERS = kdevappfrontend.h kdevmakefrontend.h kdeveditormanager.h kdevlanguagesupport.h \
			kdevversioncontrol.h kdevviewhandler.h KDevClassStore.h


METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = kdevelopcomponentviewhandler.desktop \
		   kdevelopversioncontrol.desktop \
		   kdeveloplanguagesupport.desktop \
		   kdevelopmakefrontend.desktop \
		   kdevelopappfrontend.desktop \
		   kdevelopeditormanager.desktop \
		   KDevelopClassStore.desktop

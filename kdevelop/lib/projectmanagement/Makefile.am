# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I$(top_srcdir)/kdevelop/lib/general

noinst_LTLIBRARIES = libkdevprojectmanagement.la

METASOURCES = AUTO

appwizardbase.h : appwizard.ui
	uic -o appwizardbase.h appwizard.ui
appwizardbase.cpp : appwizard.ui
	uic -i appwizardbase.h -o appwizardbase.cpp appwizard.ui

filepropspagebase.h : filepropspage.ui
	uic -o filepropspagebase.h filepropspage.ui
filepropspagebase.cpp : filepropspage.ui
	uic -i filepropspagebase.h -o filepropspagebase.cpp filepropspage.ui

libkdevprojectmanagement_la_SOURCES =  appwizard.cpp  automakeprojectspace.cpp  cppbinaryprojectplugin.cpp \
		plugin.cpp pmc.cpp project.cpp projectspace.cpp qobjectplugin.cpp \
		registeredfile.cpp  pluginloader.cpp appwizardbase.cpp filepropspagebase.cpp \
		filepropspage.cpp


noinst_HEADERS = appwizard.h  automakeprojectspace.h  cppbinaryprojectplugin.h \
				 plugin.h pmc.h project.h projectspace.h qobjectplugin.h \
				registeredfile.h  pluginloader.h appwizardbase.h filepropspagebase.h \
		filepropspage.h
#noinst_PROGRAMS = testprj

#testprj_SOURCES = testprj.cpp

# the library search path. 
#testprj_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.

#testprj_LDADD = $(top_builddir)/kdevelop/lib/projectmanagement/libkdevprojectmanagement.la \
		$(top_builddir)/kdevelop/lib/general/libkdevgeneral.la \
		$(LIB_KFILE) $(LIB_KPARTS)

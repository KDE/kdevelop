UIC = uic

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I$(top_srcdir)/kdevelop/lib/general

noinst_LTLIBRARIES = libkdevprojectmanagement.la

METASOURCES = appwizard.moc appwizardbase.moc automakeprojectspace.moc \
	      filepropspage.moc filepropspagebase.moc pmc.moc \
	      project.moc projectspace.moc qobjectplugin.moc

appwizardbase.h : $(srcdir)/appwizard.ui
	$(UIC) -o appwizardbase.h $(srcdir)/appwizard.ui
	$(MOC) appwizardbase.h -o appwizardbase.moc
appwizardbase.cpp : $(srcdir)/appwizard.ui
	$(UIC) -i appwizardbase.h -o appwizardbase.cpp $(srcdir)/appwizard.ui

filepropspagebase.h : $(srcdir)/filepropspage.ui
	$(UIC) -o filepropspagebase.h $(srcdir)/filepropspage.ui
	$(MOC) filepropspagebase.h -o filepropspagebase.moc
filepropspagebase.cpp : $(srcdir)/filepropspage.ui
	$(UIC) -i filepropspagebase.h -o filepropspagebase.cpp $(srcdir)/filepropspage.ui

EXTRA_DIST = appwizard.ui filepropspage.ui

BUILT_SOURCES = appwizardbase.h appwizardbase.cpp \
		filepropspagebase.h filepropspagebase.cpp
CLEANFILES = $(BUILT_SOURCES)

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

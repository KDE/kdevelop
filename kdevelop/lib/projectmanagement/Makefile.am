# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I$(top_srcdir)/kdevelop/lib/general

noinst_LTLIBRARIES = libkdevprojectmanagement.la

libkdevprojectmanagement_la_SOURCES = appwizard.cpp  automakeprojectspace.cpp  cppbinaryprojectplugin.cpp \
		plugin.cpp pmc.cpp project.cpp projectspace.cpp qobjectplugin.cpp filepropspage.cpp \
		registeredfile.cpp  pluginloader.cpp \
		appwizardbase.ui filepropspagebase.ui

noinst_HEADERS = appwizard.h  automakeprojectspace.h  cppbinaryprojectplugin.h \
				plugin.h pmc.h project.h projectspace.h qobjectplugin.h \
				filepropspage.h registeredfile.h  pluginloader.h \
				appwizardbase.h filepropspagebase.h

METASOURCES = AUTO

#noinst_PROGRAMS = testprj

#testprj_SOURCES = testprj.cpp

# the library search path. 
#testprj_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.

#testprj_LDADD = $(top_builddir)/kdevelop/lib/projectmanagement/libkdevprojectmanagement.la \
#		$(top_builddir)/kdevelop/lib/general/libkdevgeneral.la \
#		$(LIB_KFILE) $(LIB_KPARTS)

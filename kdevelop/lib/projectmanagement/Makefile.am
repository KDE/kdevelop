# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I$(top_srcdir)/kdevelop/lib/general

lib_LTLIBRARIES = libkdevprojectmanagement.la
libkdevprojectmanagement_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0 
libkdevprojectmanagement_la_LIBADD = $(top_builddir)/kdevelop/lib/general/libkdevelop.la \
				 $(LIB_KFILE) $(LIB_KPARTS)

METASOURCES = AUTO

libkdevprojectmanagement_la_SOURCES =  appwizard.cpp  automakeprojectspace.cpp  cppbinaryprojectplugin.cpp \
		plugin.cpp pmc.cpp project.cpp projectspace.cpp qobjectplugin.cpp \
				registeredfile.cpp  pluginloader.cpp


noinst_HEADERS = appwizard.h  automakeprojectspace.h  cppbinaryprojectplugin.h \
				 plugin.h pmc.h project.h projectspace.h qobjectplugin.h \
				registeredfile.h  pluginloader.h
noinst_PROGRAMS = testprj

testprj_SOURCES = testprj.cpp

# the library search path. 
testprj_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.

testprj_LDADD = $(top_builddir)/kdevelop/lib/general/libkdevelop.la \
		$(top_builddir)/kdevelop/lib/projectmanagement/libkdevprojectmanagement.la \
		$(LIB_KFILE)
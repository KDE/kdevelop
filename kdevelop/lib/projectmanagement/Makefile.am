# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -I$(top_srcdir)/kdevelop/lib/general -I$(top_srcdir)/kdevelop/lib/projectmanagement/KDevCompiler \
					-I$(top_srcdir)/kdevelop/lib/ComponentManager -I$(top_srcdir)/kdevelop/lib/ComponentManager/Components

noinst_LTLIBRARIES = libkdevprojectmanagement.la

libkdevprojectmanagement_la_SOURCES =  addtreedlgbase.ui appwizard.cpp  automakeprojectspace.cpp  cppbinaryprojectplugin.cpp \
		pmc.cpp project.cpp projectspace.cpp filepropspage.cpp registeredfile.cpp \
		appwizardbase.ui filepropspagebase.ui addexistingfilesbase.ui \
		addexistingfilesdlg.cpp flatdirtreeview.cpp  addtreedlg.cpp renamefiledlg.ui

libkdevprojectmanagement_la_LIBADD = $(top_builddir)/kdevelop/lib/projectmanagement/ProjectOptions/libKDevProjectOptions.la \
	$(top_builddir)/kdevelop/lib/projectmanagement/KDevCompiler/libKDevCompiler.la

noinst_HEADERS = addtreedlgbase.h appwizard.h  automakeprojectspace.h  cppbinaryprojectplugin.h \
				pmc.h project.h projectspace.h \
				filepropspage.h registeredfile.h \
				appwizardbase.h filepropspagebase.h addexistingfilesdlg.h\
				flatdirtreeview.h  addtreedlg.h

SUBDIRS = ProjectOptions KDevCompiler
				
METASOURCES = AUTO

#noinst_PROGRAMS = testprj

#testprj_SOURCES = addtreedlgbase.ui addtreedlg.cpp testprj.cpp addexistingfilesbase.ui addexistingfilesdlg.cpp  

# the library search path. 
#testprj_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.

#testprj_LDADD = $(top_builddir)/kdevelop/lib/projectmanagement/libkdevprojectmanagement.la \
#	$(top_builddir)/kdevelop/lib/general/libkdevgeneral.la \
#		$(LIB_KFILE) $(LIB_KPARTS)

projectspace.cpp: addexistingfilesbase.h renamefiledlg.h

addexistingfilesdlg.cpp: addtreedlgbase.h

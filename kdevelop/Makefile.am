SUBDIRS = widgets lib plugin parts templates kwrite_part

INCLUDES = -I$(top_srcdir)/kdevelop/lib/general \
	   -I$(top_srcdir)/kdevelop/lib/sourceinfo \
	   -I$(top_srcdir)/kdevelop/lib/projectmanagement \
	   -I$(top_srcdir)/kdevelop/widgets/qextmdi/include \
	   -I$(top_builddir)/kdevelop/lib/projectmanagement \
	   $(all_includes)

LFLAGS = -+

####### This part is very kdevelop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = kdevelop

# Which sources should be compiled for kdevelop.
kdevelop_SOURCES = main.cpp kdevelopcore.cpp newprojectdlg.cpp kdevelop.cpp \
        kdevelopfactory.cpp projectoptionsdlg.cpp \
		    newprojectdlgbase.ui

kdevelop_METASOURCES = AUTO

# the library search path. 
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.

kdevelop_LDADD = $(top_builddir)/kdevelop/lib/libkdevelop.la \
                 $(top_builddir)/kdevelop/widgets/qextmdi/src/libkdevqextmdi.la

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = kdevelop.h kdevelopfactory.h kdevelopcore.h \
                 projectoptionsdlg.h newprojectdlg.h newprojectdlgbase.h

EXTRA_DIST = kdeveloprc kdevelopui.rc\
	.directory kdevelop.desktop x-kdevelop-project.desktop kdevelop-setup.desktop \
	los-app-development_package.png lom-app-kdevelop.png los-app-kdevelop.png lom-app-kdevelop_project.png

appsdir   = $(kde_appsdir)/Development
KDE_ICON  = kdevelop kdevelop_project development_package
apps_DATA = kdevelop.desktop kdevelop-setup.desktop .directory

confdir   = $(kde_confdir)
conf_DATA = kdeveloprc

rcdir = $(kde_datadir)/kdevelop
rc_DATA = kdevelopui.rc

mimedir   = $(kde_mimedir)/application
mime_DATA = x-kdevelop-project.desktop

messages:
	$(XGETTEXT) `find . -name \*.h -o -name \*.cpp -o -name \*.cc` -o $(podir)/kdevelop.pot


# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.desktop) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)
# claim, which subdirectories you want to install
# if you don't have subdirectories, comment it
SUBDIRS = tools templates
LFLAGS = -+
####### This part is very kdevelop specific
# you can add here more. This one gets installed 
bin_PROGRAMS = kdevelop

# Which sources should be compiled for kdevelop.
kdevelop_SOURCES = main.cpp kdevelop.cpp kdevelopfactory.cpp

kdevelop_METASOURCES = kdevelop.moc

# the library search path. 
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
# the libraries to link against. Be aware of the order. First the libraries,
# that depend on the following ones.

kdevelop_LDADD = $(LIB_KHTML) -lkspell -lkfile -lfl

# this option you can leave out. Just, if you use "make dist", you need it
noinst_HEADERS = kdevelop.h kdevelopfactory.h

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

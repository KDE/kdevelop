SET(kdevdesigner_SRCS
main.cpp
kdevdesigner.cpp
)

KDE_AUTOMOC(${kdevdesigner_SRCS})

ADD_EXECUTABLE(kdevdesigner ${kdevdesigner_SRCS})



#original Makefile.am contents follow:

## set the include path for X, qt and KDE
#INCLUDES = $(all_includes)
#
## these are the headers for your project
#noinst_HEADERS = kdevdesigner.h
#
## let automoc handle all of the meta source files (moc)
#METASOURCES = AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kdevdesigner.pot
#
#
## this Makefile creates both a KPart application and a KPart
##########################################################################
## APPLICATION SECTION
##########################################################################
## this is the program that gets installed.  it's name is used for all
## of the other Makefile.am variables
#bin_PROGRAMS = kdevdesigner
#
## the application source, library search path, and link libraries
#kdevdesigner_SOURCES = main.cpp kdevdesigner.cpp
#kdevdesigner_LDFLAGS = $(KDE_RPATH) $(all_libraries)
#kdevdesigner_LDADD   = $(LIB_KPARTS)
#
## this is where the desktop file will go 
##shelldesktopdir = $(kde_appsdir)/Development
##shelldesktop_DATA = kdevdesigner.desktop
#xdg_apps_DATA = kdevdesigner.desktop
#
## this is where the shell's XML-GUI resource file goes
#shellrcdir = $(kde_datadir)/kdevdesigner
#shellrc_DATA = kdevdesigner_shell.rc
#
##########################################################################
## KPART SECTION
##########################################################################
#
## the Part's source, library search path, and link libraries
#
#
## this is where the desktop file will go 
#partdesktopdir   = $(kde_servicesdir)
#
## this is where the part's XML-GUI resource file goes
#partrcdir   = $(kde_datadir)/kdevdesignerpart

#######################
#previous CMakeLists.txt contents follow:


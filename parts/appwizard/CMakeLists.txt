SUBDIRS( imports common licenses )

SET(kdevappwizard_PART_SRCS
appwizardfactory.cpp
appwizardpart.cpp
appwizarddlg.cpp
importdlg.cpp
filepropspage.cpp
misc.cpp
kdevfile.cpp
kdevlicense.cpp
profilesupport.cpp
)

KDE_AUTOMOC(${kdevappwizard_PART_SRCS})

SET( kdevappwizard_UI
filepropspagebase.ui
appwizarddlgbase.ui
importdlgbase.ui
vcs_form.ui
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/external/ ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/ ${CMAKE_SOURCE_DIR}/lib/interfaces/extras/ ${CMAKE_SOURCE_DIR}/lib/widgets/)

KDE_ADD_UI_FILES(kdevappwizard_PART_SRCS ${kdevappwizard_UI} )

ADD_LIBRARY(kdevappwizard SHARED ${kdevappwizard_PART_SRCS})



#original Makefile.am contents follow:

#
## Here resides the app wizard part.
#
#SUBDIRS = . imports common licenses
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
#	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevappwizard.la
#libkdevappwizard_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevappwizard_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/interfaces/extras/libkdevextras.la $(top_builddir)/lib/util/libkdevutil.la \
#	$(top_builddir)/lib/widgets/propeditor/libkdevpropertyeditor.la
#
#libkdevappwizard_la_SOURCES = appwizardfactory.cpp appwizardpart.cpp \
#		appwizarddlg.cpp importdlg.cpp filepropspagebase.ui filepropspage.cpp \
#		appwizarddlgbase.ui importdlgbase.ui misc.cpp vcs_form.ui\
#		kdevfile.cpp kdevlicense.cpp profilesupport.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevappwizard.desktop
#
#rcdir = $(kde_datadir)/kdevappwizard
#rc_DATA = kdevappwizard.rc
#noinst_HEADERS = profilesupport.h

#######################
#previous CMakeLists.txt contents follow:

#SUBDIRS( . imports common licenses )
#
#SET(kdevappwizard_PART_SRCS
#appwizardfactory.cpp
#appwizardpart.cpp
#appwizarddlg.cpp
#importdlg.cpp
#filepropspage.cpp
#misc.cpp
#kdevfile.cpp
#kdevlicense.cpp
#profilesupport.cpp
#)
#
#KDE_AUTOMOC(${kdevappwizard_PART_SRCS})
#
#SET( kdevappwizard_UI
#filepropspagebase.ui
#appwizarddlgbase.ui
#importdlgbase.ui
#vcs_form.ui
#)
#
#KDE_ADD_UI_FILES(kdevappwizard_PART_SRCS ${kdevappwizard_UI} )
#
#ADD_LIBRARY(kdevappwizard SHARED ${kdevappwizard_PART_SRCS})
#

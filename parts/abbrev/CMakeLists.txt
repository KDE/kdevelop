#the source files for this part
SET(kdevabbrev_PART_SRCS
abbrevpart.cpp
abbrevconfigwidget.cpp
addtemplatedlg.cpp
)

#the ui files for this part
SET( kdevabbrev_UI
abbrevconfigwidgetbase.ui
addtemplatedlgbase.ui
)

#automatically generated the rules for the moc files for the sources
KDE_AUTOMOC(${kdevabbrev_PART_SRCS})

#add the ui files to the project
KDE_ADD_UI_FILES(kdevabbrev_PART_SRCS ${kdevabbrev_UI} )

#create a shared library/module/plugin
ADD_LIBRARY(kdevabbrev MODULE ${kdevabbrev_PART_SRCS})

#link it to some kde libs
TARGET_LINK_LIBRARIES(kdevabbrev ${KDEV_PART_LIBS})

#create the libtool la file for this plugin
KDE_CREATE_LIBTOOL_FILE(kdevabbrev)

#install rules, relative to CMAKE_INSTALL_PREFIX
INSTALL_TARGETS(/lib/kde3 kdevabbrev)
INSTALL_FILES(/share/apps/kdevabbrev FILES kdevabbrev.rc)
INSTALL_FILES(/share/services FILES kdevabbrev.desktop)
INSTALL_FILES(/share/apps/kdevabbrev/sources FILES qt_classes cpp_keywords)

#SET_TARGET_PROPERTIES(kdevabbrev PROPERTIES PREFIX "")


#original Makefile.am contents follow:

## Here resides the abbrev part
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevabbrev.la
#libkdevabbrev_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevabbrev_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevabbrev_la_SOURCES = abbrevpart.cpp abbrevconfigwidget.cpp abbrevconfigwidgetbase.ui addtemplatedlg.cpp addtemplatedlgbase.ui
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevabbrev.desktop
#
#sourcesdir = $(kde_datadir)/kdevabbrev/sources
#sources_DATA = qt_classes cpp_keywords
#
#rcdir = $(kde_datadir)/kdevabbrev
#rc_DATA = kdevabbrev.rc

SUBDIRS( file-templates )

SET(kdevfilecreate_PART_SRCS
filecreate_part.cpp
filecreate_listitem.cpp
filecreate_widget3.cpp
filecreate_widget2.cpp
filecreate_filetype.cpp
filecreate_filedialog.cpp
fcconfigwidget.cpp
fctypeedit.cpp
fctemplateedit.cpp
filecreate_newfile.cpp
)

SET( kdevfilecreate_UI
fcconfigwidgetbase.ui
fctypeeditbase.ui
fctemplateeditbase.ui
)

KDE_AUTOMOC(${kdevfilecreate_PART_SRCS})

KDE_ADD_UI_FILES(kdevfilecreate_PART_SRCS ${kdevfilecreate_UI} )

KDE_ADD_MOC_FILES(kdevfilecreate_PART_SRCS filecreate_typechoosersig.h)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/ ${CMAKE_CURRENT_SOURCE_DIR})

ADD_LIBRARY(kdevfilecreate SHARED ${kdevfilecreate_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevfilecreate ${KDEV_PART_LIBS})


#original Makefile.am contents follow:

##
#
#SUBDIRS = file-templates
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevfilecreate.la
#libkdevfilecreate_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevfilecreate_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevfilecreate_la_SOURCES = filecreate_part.cpp filecreate_listitem.cpp filecreate_widget3.cpp filecreate_widget2.cpp filecreate_filetype.cpp filecreate_filedialog.cpp fcconfigwidgetbase.ui fcconfigwidget.cpp fctypeeditbase.ui fctypeedit.cpp fctemplateeditbase.ui fctemplateedit.cpp filecreate_newfile.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevfilecreate.desktop
#
#rcdir = $(kde_datadir)/kdevfilecreate
#rc_DATA = kdevpart_filecreate.rc template-info.xml

#######################
#previous CMakeLists.txt contents follow:

#SUBDIRS( file-templates )
#
#SET(kdevfilecreate_PART_SRCS
#filecreate_part.cpp
#filecreate_listitem.cpp
#filecreate_widget3.cpp
#filecreate_widget2.cpp
#filecreate_filetype.cpp
#filecreate_filedialog.cpp
#fcconfigwidget.cpp
#fctypeedit.cpp
#fctemplateedit.cpp
#filecreate_newfile.cpp
#)
#
#KDE_AUTOMOC(${kdevfilecreate_PART_SRCS})
#
#SET( kdevfilecreate_UI
#fcconfigwidgetbase.ui
#fctypeeditbase.ui
#fctemplateeditbase.ui
#)
#
#KDE_ADD_UI_FILES(kdevfilecreate_PART_SRCS ${kdevfilecreate_UI} )
#
#ADD_LIBRARY(kdevfilecreate SHARED ${kdevfilecreate_PART_SRCS})
#

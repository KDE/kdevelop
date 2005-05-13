SET(kdevvcsmanager_PART_SRCS
vcsmanagerpart.cpp
vcsmanagerprojectconfig.cpp
)

SET( kdevvcsmanager_UI
vcsmanagerprojectconfigbase.ui
)

KDE_AUTOMOC(${kdevvcsmanager_PART_SRCS})

KDE_ADD_UI_FILES(kdevvcsmanager_PART_SRCS ${kdevvcsmanager_UI} )

ADD_LIBRARY(kdevvcsmanager MODULE ${kdevvcsmanager_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevvcsmanager ${KDEV_PART_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevvcsmanager)

INSTALL_TARGETS(/lib/kde3 kdevvcsmanager)

INSTALL_FILES(/share/servicetypes FILES kdevvcsmanager.desktop)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevvcsmanager.la
#libkdevvcsmanager_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevvcsmanager_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevvcsmanager_la_SOURCES = vcsmanagerpart.cpp \
#	vcsmanagerprojectconfigbase.ui vcsmanagerprojectconfig.cpp
#
#METASOURCES = AUTO
#
#appsharedir = $(kde_datadir)/kdevvcsmanager
#appshare_DATA =
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevvcsmanager.desktop
#
#rcdir = $(kde_datadir)/kdevvcsmanager
#


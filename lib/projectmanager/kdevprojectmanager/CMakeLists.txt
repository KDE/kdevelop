
INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/buildtools/projectmanager/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets/propeditor ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR})

SET(kdevprojectmanager_PART_SRCS
../kdevprojectmanager_part.cpp
../kdevprojectmanager_widget.cpp
../importprojectjob.cpp
)

KDE_AUTOMOC(${kdevprojectmanager_PART_SRCS})

ADD_LIBRARY(kdevprojectmanager MODULE ${kdevprojectmanager_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevprojectmanager ${QT_AND_KDECORE_LIBS} kdeui kio kdevelop projectmanager_interfaces)

KDE_CREATE_LIBTOOL_FILE(kdevprojectmanager)

INSTALL_TARGETS(/lib/kde3 kdevprojectmanager)
INSTALL_FILES(/share/services FILES ../kdevprojectmanager.desktop )
INSTALL_FILES(/share/apps/kdevprojectmanger FILES ../kdevprojectmanager.rc )

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/buildtools/projectmanager/interfaces \
#	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/lib/widgets/propeditor $(all_includes)
#
#SUBDIRS = interfaces . importers builders
#
#kde_module_LTLIBRARIES = libkdevprojectmanager.la
#libkdevprojectmanager_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevprojectmanager_la_LIBADD = \
#	$(top_builddir)/buildtools/projectmanager/interfaces/libprojectmanager_interfaces.la $(top_builddir)/lib/libkdevelop.la
#
#libkdevprojectmanager_la_SOURCES = \
#    kdevprojectmanager_part.cpp \
#    kdevprojectmanager_widget.cpp \
#    importprojectjob.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevprojectmanager.desktop
#
#rcdir = $(kde_datadir)/kdevprojectmanager
#rc_DATA = \
#    kdevprojectmanager.rc
#
#DOXYGEN_EMPTY = YES
#include ../../Doxyfile.am

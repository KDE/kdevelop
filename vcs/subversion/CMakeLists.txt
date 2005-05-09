SUBDIRS( integrator )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_CURRENT_SOURCE_DIR})

SET(kdevsubversion_PART_SRCS
subversion_part.cpp
subversion_widget.cpp
subversion_core.cpp
subversionoptionswidgetimpl.cpp
subversion_fileinfo.cpp
)

SET( kdevsubversion_UI
svn_co.ui
subversionprojectwidget.ui
subversionoptionswidget.ui
)

SET( kdevsubversion_DCOP_SRCS
subversion_core.h
subversion_fileinfo.h
)

KDE_AUTOMOC(${kdevsubversion_PART_SRCS})

KDE_ADD_UI_FILES(kdevsubversion_PART_SRCS ${kdevsubversion_UI} )

KDE_ADD_DCOP_SKELS(kdevsubversion_PART_SRCS ${kdevsubversion_DCOP_SRCS})

ADD_LIBRARY(kdevsubversion SHARED ${kdevsubversion_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevsubversion ${QT_LIBRARIES} kdecore kdeui kio kparts DCOP kdevelop kdevextensions)


#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/interfaces/extensions $(all_includes)
#
#if include_vcscompat
#NETACCESS_LIB = $(top_builddir)/vcs/compat/netaccess/libnetaccess.la
#endif
#
#kde_module_LTLIBRARIES = libkdevsubversion.la
#libkdevsubversion_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevsubversion_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/extensions/libkdevextensions.la $(NETACCESS_LIB)
#
#libkdevsubversion_la_SOURCES = subversion_core.skel subversion_part.cpp subversion_widget.cpp subversion_core.cpp svn_co.ui subversionprojectwidget.ui subversionoptionswidget.ui subversionoptionswidgetimpl.cpp subversion_fileinfo.cpp subversion_fileinfo.skel
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevsubversion.desktop
#
#rcdir = $(kde_datadir)/kdevsubversion
#SUBDIRS = integrator

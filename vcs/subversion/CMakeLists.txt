SUBDIRS( integrator )

SET(kdevsubversion_PART_SRCS
subversion_core.skel
subversion_part.cpp
subversion_widget.cpp
subversion_core.cpp
subversionoptionswidgetimpl.cpp
subversion_fileinfo.cpp
subversion_fileinfo.skel
)

KDE_AUTOMOC(${kdevsubversion_PART_SRCS})

SET(
svn_co.ui
subversionprojectwidget.ui
subversionoptionswidget.ui
)

KDE_ADD_UI_FILES(kdevsubversion_PART_SRCS ${kdevsubversion_UI}

ADD_LIBRARY(kdevsubversion SHARED ${kdevsubversion_PART_SRCS})



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

#######################
#previous CMakeLists.txt contents follow:


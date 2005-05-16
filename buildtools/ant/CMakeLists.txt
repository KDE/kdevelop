INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/buildtools/lib/base ${CMAKE_SOURCE_DIR}/buildtools/lib/widgets ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevantproject_PART_SRCS
antprojectpart.cpp
)

SET( kdevantproject_UI
antoptionswidget.ui
classpathwidget.ui
)

KDE_AUTOMOC(${kdevantproject_PART_SRCS})

KDE_ADD_UI_FILES(kdevantproject_PART_SRCS ${kdevantproject_UI} )

ADD_LIBRARY(kdevantproject MODULE ${kdevantproject_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevantproject ${KDEV_BUILDTOOLS_LIBS} )

KDE_CREATE_LIBTOOL_FILE(kdevantproject)

INSTALL_TARGETS(/lib/kde3 kdevantproject)

INSTALL_FILES(/share/services FILES kdevantproject.desktop )
INSTALL_FILES(/share/apps/kdevantproject FILES kdevantproject.rc )

#original Makefile.am contents follow:

## Here resides the ant project part.
#
#INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
#	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_builddir)/buildtools/lib/widgets \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = libkdevantproject.la
#libkdevantproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevantproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la $(top_builddir)/buildtools/lib/base/libkdevbuildbase.la
#
#libkdevantproject_la_SOURCES = antprojectpart.cpp antoptionswidget.ui classpathwidget.ui
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevantproject.desktop
#
#rcdir = $(kde_datadir)/kdevantproject
#rc_DATA = kdevantproject.rc

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/buildtools/lib/base ${CMAKE_SOURCE_DIR}/buildtools/lib/widgets ${CMAKE_BINARY_DIR}/buildtools/lib/widgets ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

SET(kdevcustomproject_PART_SRCS
customprojectpart.cpp
custombuildoptionswidget.cpp
custommakeconfigwidget.cpp
)

SET( kdevcustomproject_UI
custombuildoptionswidgetbase.ui
custommakeconfigwidgetbase.ui
)

KDE_AUTOMOC(${kdevcustomproject_PART_SRCS})

KDE_ADD_UI_FILES(kdevcustomproject_PART_SRCS ${kdevcustomproject_UI} )

ADD_LIBRARY(kdevcustomproject MODULE ${kdevcustomproject_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevcustomproject ${KDEV_BUILDTOOLS_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevcustomproject)

INSTALL_TARGETS(/lib/kde3 kdevcustomproject)
INSTALL_FILES(/share/services FILES kdevcustomproject.desktop )
INSTALL_FILES(/share/apps/kdevcustomproject FILES kdevcustomproject.rc )

#original Makefile.am contents follow:

## Here resides the custom project part.
#
#INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
#	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_builddir)/buildtools/lib/widgets \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = libkdevcustomproject.la
#libkdevcustomproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevcustomproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la $(top_builddir)/buildtools/lib/base/libkdevbuildbase.la
#
#libkdevcustomproject_la_SOURCES = customprojectpart.cpp custombuildoptionswidgetbase.ui custombuildoptionswidget.cpp custommakeconfigwidgetbase.ui custommakeconfigwidget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevcustomproject.desktop
#
#rcdir = $(kde_datadir)/kdevcustomproject
#rc_DATA = kdevcustomproject.rc

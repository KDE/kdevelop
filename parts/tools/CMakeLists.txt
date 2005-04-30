SET(kdevtools_PART_SRCS
tools_part.cpp
toolsconfig.cpp
toolsconfigwidget.cpp
kapplicationtree.cpp
)

SET( kdevtools_UI
toolsconfigwidgetbase.ui
addtooldlg.ui
)

KDE_AUTOMOC(${kdevtools_PART_SRCS})

KDE_ADD_UI_FILES(kdevtools_PART_SRCS ${kdevtools_UI} )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/)

ADD_LIBRARY(kdevtools SHARED ${kdevtools_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevtools ${KDEV_PART_LIBS})

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevtools.la
#libkdevtools_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevtools_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevtools_la_SOURCES = tools_part.cpp toolsconfig.cpp toolsconfigwidgetbase.ui toolsconfigwidget.cpp kapplicationtree.cpp addtooldlg.ui
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevtools.desktop
#
#rc_DATA = kdevpart_tools.rc
#rcdir = $(kde_datadir)/kdevtools

SET(kdevpartexplorer_PART_SRCS
partexplorerform.cpp
partexplorer_plugin.cpp
)

KDE_AUTOMOC(${kdevpartexplorer_PART_SRCS})

SET(
partexplorerformbase.ui
)

KDE_ADD_UI_FILES(kdevpartexplorer_PART_SRCS ${kdevpartexplorer_UI}

ADD_LIBRARY(kdevpartexplorer SHARED ${kdevpartexplorer_PART_SRCS})



#original Makefile.am contents follow:

## Here resides the PartExplorer plugin
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevpartexplorer.la
#libkdevpartexplorer_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevpartexplorer_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevpartexplorer_la_SOURCES = partexplorerformbase.ui partexplorerform.cpp partexplorer_plugin.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevpartexplorer.desktop
#
#rcdir = $(kde_datadir)/kdevpartexplorer
#rc_DATA = kdevpartexplorer.rc

#######################
#previous CMakeLists.txt contents follow:


SET(kdevvisualboyadvance_PART_SRCS
visualboyadvance_part.cpp
vbaconfigwidget.cpp
)

KDE_AUTOMOC(${kdevvisualboyadvance_PART_SRCS})

SET(
vbaconfigwidgetbase.ui
)

KDE_ADD_UI_FILES(kdevvisualboyadvance_PART_SRCS ${kdevvisualboyadvance_UI}

ADD_LIBRARY(kdevvisualboyadvance SHARED ${kdevvisualboyadvance_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevvisualboyadvance.la
#libkdevvisualboyadvance_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevvisualboyadvance_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevvisualboyadvance_la_SOURCES = visualboyadvance_part.cpp vbaconfigwidgetbase.ui vbaconfigwidget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevvisualboyadvance.desktop
#
#rcdir = $(kde_datadir)/kdevvisualboyadvance
#rc_DATA = kdevpart_visualboyadvance.rc
#

#######################
#previous CMakeLists.txt contents follow:


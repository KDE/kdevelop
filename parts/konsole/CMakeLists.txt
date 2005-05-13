SET(kdevkonsoleview_PART_SRCS
konsoleviewpart.cpp
konsoleviewwidget.cpp
)

KDE_AUTOMOC(${kdevkonsoleview_PART_SRCS})

ADD_LIBRARY(kdevkonsoleview MODULE ${kdevkonsoleview_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevkonsoleview ${KDEV_PART_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevkonsoleview)

INSTALL_TARGETS(/lib/kde3 kdevkonsoleview)

INSTALL_FILES(/share/servicetypes FILES kdevkonsoleview.desktop)

#original Makefile.am contents follow:

## Here resides the konsole view part.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevkonsoleview.la
#libkdevkonsoleview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevkonsoleview_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevkonsoleview_la_SOURCES = konsoleviewpart.cpp konsoleviewwidget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevkonsoleview.desktop
#


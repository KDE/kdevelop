SET(kdevannotation_PART_SRCS
annotationpart.cpp
annotationwidget.cpp
annotationglobalconfig.cpp
annotationprojectconfig.cpp
annotationitem.cpp
)

KDE_AUTOMOC(${kdevannotation_PART_SRCS})

SET( kdevannotation_UI
annotationglobalconfigbase.ui
annotationprojectconfigbase.ui
)

KDE_ADD_UI_FILES(kdevannotation_PART_SRCS ${kdevannotation_UI} )

ADD_LIBRARY(kdevannotation MODULE ${kdevannotation_PART_SRCS})

KDE_CREATE_LIBTOOL_FILE(kdevannotation)

INSTALL_TARGETS(/lib/kde3 kdevannotation)

INSTALL_FILES(/share/servicetypes FILES kdevannotation.desktop)
INSTALL_FILES(/share/apps/kdevannotation FILES kdevannotation.rc)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevannotation.la
#libkdevannotation_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevannotation_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevannotation_la_SOURCES = annotationpart.cpp annotationwidget.cpp \
#		annotationglobalconfigbase.ui annotationglobalconfig.cpp 	annotationprojectconfigbase.ui \
#	annotationprojectconfig.cpp annotationitem.cpp
#
#METASOURCES = AUTO
#
#appsharedir = $(kde_datadir)/kdevannotation
#appshare_DATA =
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevannotation.desktop
#
#rcdir = $(kde_datadir)/kdevannotation
#rc_DATA = kdevannotation.rc
#noinst_HEADERS = annotationitem.h


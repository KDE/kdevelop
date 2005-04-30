SET(kdevscripting_PART_SRCS
scriptingpart.cpp
scriptingglobalconfig.cpp
)

SET( kdevscripting_UI
scriptingglobalconfigbase.ui
)

KDE_AUTOMOC(${kdevscripting_PART_SRCS})

KDE_ADD_UI_FILES(kdevscripting_PART_SRCS ${kdevscripting_UI} )

ADD_LIBRARY(kdevscripting SHARED ${kdevscripting_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevscripting ${KDEV_PART_LIBS})

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevscripting.la
#libkdevscripting_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevscripting_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevscripting_la_SOURCES = scriptingpart.cpp scriptingglobalconfigbase.ui \
#	scriptingglobalconfig.cpp
#
#METASOURCES = AUTO
#
#appsharedir = $(kde_datadir)/kdevscripting
#appshare_DATA =
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevscripting.desktop
#
#rcdir = $(kde_datadir)/kdevscripting
#rc_DATA = kdevscripting.rc
#noinst_HEADERS = scriptingglobalconfig.h  scriptingpart.h

#######################
#previous CMakeLists.txt contents follow:

#SET(kdevscripting_PART_SRCS
#scriptingpart.cpp
#scriptingglobalconfig.cpp
#)
#
#KDE_AUTOMOC(${kdevscripting_PART_SRCS})
#
#SET( kdevscripting_UI
#scriptingglobalconfigbase.ui
#)
#
#KDE_ADD_UI_FILES(kdevscripting_PART_SRCS ${kdevscripting_UI} )
#
#ADD_LIBRARY(kdevscripting SHARED ${kdevscripting_PART_SRCS})
#

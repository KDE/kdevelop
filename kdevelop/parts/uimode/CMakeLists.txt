SET(kdevuichooser_PART_SRCS
uichooser_part.cpp
uichooser_widget.cpp
)

KDE_AUTOMOC(${kdevuichooser_PART_SRCS})

SET( kdevuichooser_UI
uichooser.ui
)

KDE_ADD_UI_FILES(kdevuichooser_PART_SRCS ${kdevuichooser_UI} )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/qextmdi/)

ADD_LIBRARY(kdevuichooser SHARED ${kdevuichooser_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = $(KDEVMDI_INCLUDES) -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevuichooser.la
#libkdevuichooser_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevuichooser_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevuichooser_la_SOURCES = uichooser_part.cpp uichooser.ui uichooser_widget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevuichooser.desktop


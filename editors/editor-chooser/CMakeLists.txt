SET(kdeveditorchooser_PART_SRCS
editorchooser_part.cpp
editorchooser_widget.cpp
)

KDE_AUTOMOC(${kdeveditorchooser_PART_SRCS})

SET(
editchooser.ui
)

KDE_ADD_UI_FILES(kdeveditorchooser_PART_SRCS ${kdeveditorchooser_UI}

ADD_LIBRARY(kdeveditorchooser SHARED ${kdeveditorchooser_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdeveditorchooser.la
#libkdeveditorchooser_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdeveditorchooser_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdeveditorchooser_la_SOURCES = editorchooser_part.cpp editchooser.ui editorchooser_widget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdeveditorchooser.desktop
#

#######################
#previous CMakeLists.txt contents follow:


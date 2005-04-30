INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdeveditorchooser_PART_SRCS
editorchooser_part.cpp
editorchooser_widget.cpp
)

SET( kdeveditorchooser_UI
editchooser.ui
)

KDE_AUTOMOC(${kdeveditorchooser_PART_SRCS})

KDE_ADD_UI_FILES(kdeveditorchooser_PART_SRCS ${kdeveditorchooser_UI} )

ADD_LIBRARY(kdeveditorchooser SHARED ${kdeveditorchooser_PART_SRCS})

TARGET_LINK_LIBRARIES(kdeveditorchooser ${QT_LIBRARIES} kdecore kdeui kio ${CMAKE_BINARY_DIR}/lib/libkdevelop.so)

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

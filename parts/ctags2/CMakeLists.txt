SET(kdevctags2_PART_SRCS
ctags2_part.cpp
readtags.c
ctags2_widget.cpp
tags.cpp
ctagskinds.cpp
ctags2_settingswidget.cpp
)

KDE_AUTOMOC(${kdevctags2_PART_SRCS})

SET( kdevctags2_UI
ctags2_widgetbase.ui
ctags2_settingswidgetbase.ui
)

KDE_ADD_UI_FILES(kdevctags2_PART_SRCS ${kdevctags2_UI} )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/)

ADD_LIBRARY(kdevctags2 SHARED ${kdevctags2_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevctags2.la
#libkdevctags2_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevctags2_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevctags2_la_SOURCES = ctags2_part.cpp readtags.c ctags2_widgetbase.ui \
#	ctags2_widget.cpp tags.cpp ctagskinds.cpp ctags2_settingswidgetbase.ui \
#	ctags2_settingswidget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevctags2.desktop
#
#rc_DATA = kdevpart_ctags2.rc
#rcdir = $(kde_datadir)/kdevctags2
#
#noinst_HEADERS = ctags2_widget.h tags.h readtags.h ctagskinds.h \
#	ctags2_settingswidget.h

SET(kdevsnippet_PART_SRCS
snippet_part.cpp
snippet_widget.cpp
snippetitem.cpp
snippetsettings.cpp
snippetconfig.cpp
)

KDE_AUTOMOC(${kdevsnippet_PART_SRCS})

SET( kdevsnippet_UI
snippetdlg.ui
snippetsettingsbase.ui
)

KDE_ADD_UI_FILES(kdevsnippet_PART_SRCS ${kdevsnippet_UI} )

ADD_LIBRARY(kdevsnippet SHARED ${kdevsnippet_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevsnippet.la
#libkdevsnippet_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevsnippet_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevsnippet_la_SOURCES = snippet_part.cpp snippet_widget.cpp snippetdlg.ui snippetitem.cpp snippetsettingsbase.ui snippetsettings.cpp snippetconfig.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevsnippet.desktop
#
#rcdir = $(kde_datadir)/kdevsnippet
#
#noinst_HEADERS = snippetdlg.h snippetitem.h snippetsettingsbase.h snippetsettings.h snippetconfig.h

#######################
#previous CMakeLists.txt contents follow:

#SET(kdevsnippet_PART_SRCS
#snippet_part.cpp
#snippet_widget.cpp
#snippetitem.cpp
#snippetsettings.cpp
#snippetconfig.cpp
#)
#
#KDE_AUTOMOC(${kdevsnippet_PART_SRCS})
#
#SET( kdevsnippet_UI
#snippetdlg.ui
#snippetsettingsbase.ui
#)
#
#KDE_ADD_UI_FILES(kdevsnippet_PART_SRCS ${kdevsnippet_UI} )
#
#ADD_LIBRARY(kdevsnippet SHARED ${kdevsnippet_PART_SRCS})
#

SUBDIRS( template subclassing_template )

SET(kdevkjssupport_PART_SRCS
kjssupport_part.cpp
kjsproblems.cpp
jscodecompletion.cpp
subclassingdlg.cpp
)

KDE_AUTOMOC(${kdevkjssupport_PART_SRCS})

SET( kdevkjssupport_UI
subclassingdlgbase.ui
)

KDE_ADD_UI_FILES(kdevkjssupport_PART_SRCS ${kdevkjssupport_UI} )

ADD_LIBRARY(kdevkjssupport SHARED ${kdevkjssupport_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(kde_includes)/kdevelop $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevkjssupport.la
#libkdevkjssupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevkjssupport_la_LIBADD = -lkjsembed -lkdevelop
#libkdevkjssupport_la_SOURCES = kjssupport_part.cpp kjsproblems.cpp jscodecompletion.cpp subclassingdlg.cpp subclassingdlgbase.ui
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevkjssupport.desktop
#
#mimesrcdir = $(kde_mimedir)/text
#mimesrc_DATA = x-javascript-source.desktop
#
#EXTRA_DIST = $(mimesrc_DATA)
#
#rcdir = $(kde_datadir)/kdevkjssupport
#rc_DATA = kdevkjssupport.rc
#
#SUBDIRS = template subclassing_template
#noinst_HEADERS =  kjsproblems.h

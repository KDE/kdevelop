SET(kdevbookmarks_PART_SRCS
bookmarks_part.cpp
bookmarks_widget.cpp
bookmarks_settings.cpp
bookmarks_config.cpp
)

KDE_AUTOMOC(${kdevbookmarks_PART_SRCS})

SET( kdevbookmarks_UI
bookmarks_settings_base.ui
)

KDE_ADD_UI_FILES(kdevbookmarks_PART_SRCS ${kdevbookmarks_UI} )

ADD_LIBRARY(kdevbookmarks MODULE ${kdevbookmarks_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevbookmarks ${KDEV_PART_LIBS} )

KDE_CREATE_LIBTOOL_FILE(kdevbookmarks)

INSTALL_TARGETS(/lib/kde3 kdevbookmarks)

INSTALL_FILES(/share/servicetypes FILES kdevbookmarks.desktop)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevbookmarks.la
#libkdevbookmarks_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevbookmarks_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevbookmarks_la_SOURCES = bookmarks_part.cpp bookmarks_widget.cpp bookmarks_settings_base.ui bookmarks_settings.cpp bookmarks_config.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevbookmarks.desktop
#
#noinst_HEADERS =  bookmarks_settings.h bookmarks_config.h

SET(kdevfileselector_PART_SRCS
fileselector_part.cpp
fileselector_widget.cpp
kactionselector.cpp
kbookmarkhandler.cpp
)

KDE_AUTOMOC(${kdevfileselector_PART_SRCS})

ADD_LIBRARY(kdevfileselector SHARED ${kdevfileselector_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevfileselector ${KDEV_PART_LIBS})


#original Makefile.am contents follow:

## Here resides the file selector part.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/sourceinfo $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevfileselector.la
#libkdevfileselector_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevfileselector_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevfileselector_la_SOURCES = fileselector_part.cpp fileselector_widget.cpp kactionselector.cpp kbookmarkhandler.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevfileselector.desktop
#
## not used currently
##rcdir = $(kde_datadir)/kdevfileselector
##rc_DATA = kdevfileselector.rc

#######################
#previous CMakeLists.txt contents follow:

#SET(kdevfileselector_PART_SRCS
#fileselector_part.cpp
#fileselector_widget.cpp
#kactionselector.cpp
#kbookmarkhandler.cpp
#)
#
#KDE_AUTOMOC(${kdevfileselector_PART_SRCS})
#
#ADD_LIBRARY(kdevfileselector SHARED ${kdevfileselector_PART_SRCS})
#

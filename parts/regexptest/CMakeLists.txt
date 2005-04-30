SET(kdevregexptest_PART_SRCS
regexptestpart.cpp
regexptestdlg.cpp
)

SET( kdevregexptest_UI
regexptestdlgbase.ui
)

KDE_AUTOMOC(${kdevregexptest_PART_SRCS})

KDE_ADD_UI_FILES(kdevregexptest_PART_SRCS ${kdevregexptest_UI} )

ADD_LIBRARY(kdevregexptest SHARED ${kdevregexptest_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevregexptest ${KDEV_PART_LIBS})


#original Makefile.am contents follow:

## Here resides the regexptest part
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevregexptest.la
#libkdevregexptest_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevregexptest_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevregexptest_la_SOURCES = regexptestpart.cpp regexptestdlg.cpp regexptestdlgbase.ui
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevregexptest.desktop
#
#rcdir = $(kde_datadir)/kdevregexptest
#rc_DATA = kdevregexptest.rc

#######################
#previous CMakeLists.txt contents follow:

#SET(kdevregexptest_PART_SRCS
#regexptestpart.cpp
#regexptestdlg.cpp
#)
#
#KDE_AUTOMOC(${kdevregexptest_PART_SRCS})
#
#SET( kdevregexptest_UI
#regexptestdlgbase.ui
#)
#
#KDE_ADD_UI_FILES(kdevregexptest_PART_SRCS ${kdevregexptest_UI} )
#
#ADD_LIBRARY(kdevregexptest SHARED ${kdevregexptest_PART_SRCS})
#

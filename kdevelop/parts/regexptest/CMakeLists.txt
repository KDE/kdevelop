SET(kdevregexptest_PART_SRCS
regexptestpart.cpp
regexptestdlg.cpp
)

SET( kdevregexptest_UI
regexptestdlgbase.ui
)

KDE_AUTOMOC(${kdevregexptest_PART_SRCS})

KDE_ADD_UI_FILES(kdevregexptest_PART_SRCS ${kdevregexptest_UI} )

ADD_LIBRARY(kdevregexptest MODULE ${kdevregexptest_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevregexptest ${KDEV_PART_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevregexptest)

INSTALL_TARGETS(/lib/kde3 kdevregexptest)

INSTALL_FILES(/share/servicetypes FILES kdevregexptest.desktop )
INSTALL_FILES(/share/apps/kdevregexptest FILES kdevregexptest.rc)

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


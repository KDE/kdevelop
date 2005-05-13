SET(kdevdiff_PART_SRCS
diffpart.cpp
diffdlg.cpp
diffwidget.cpp
)

KDE_AUTOMOC(${kdevdiff_PART_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/)

ADD_LIBRARY(kdevdiff MODULE ${kdevdiff_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevdiff ${KDEV_PART_LIBS} )

KDE_CREATE_LIBTOOL_FILE(kdevdiff)

INSTALL_TARGETS(/lib/kde3 kdevdiff)

INSTALL_FILES(/share/servicetypes FILES kdevdiff.desktop)
INSTALL_FILES(/share/apps/kdevdiff FILES kdevdiff.rc)

#original Makefile.am contents follow:

## Here resides the valgrind part
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevdiff.la
#libkdevdiff_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevdiff_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevdiff_la_SOURCES = diffpart.cpp diffdlg.cpp diffwidget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevdiff.desktop
#
#rcdir = $(kde_datadir)/kdevdiff
#rc_DATA = kdevdiff.rc

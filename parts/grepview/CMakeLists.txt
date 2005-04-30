SET(kdevgrepview_PART_SRCS
grepviewpart.cpp
grepviewwidget.cpp
grepdlg.cpp
)

KDE_AUTOMOC(${kdevgrepview_PART_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/widgets/)

ADD_LIBRARY(kdevgrepview SHARED ${kdevgrepview_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevgrepview ${KDEV_PART_LIBS} kdevwidgets)

#original Makefile.am contents follow:

## Here resides the grep view part.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevgrepview.la
#libkdevgrepview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevgrepview_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/widgets/libkdevwidgets.la $(LIB_KHTML)
#
#libkdevgrepview_la_SOURCES = grepviewpart.cpp grepviewwidget.cpp grepdlg.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevgrepview.desktop
#
#rcdir = $(kde_datadir)/kdevgrepview
#rc_DATA = kdevgrepview.rc

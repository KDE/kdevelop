SET(kdevopenwith_PART_SRCS
openwithpart.cpp
)

KDE_AUTOMOC(${kdevopenwith_PART_SRCS})

ADD_LIBRARY(kdevopenwith MODULE ${kdevopenwith_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevopenwith ${KDEV_PART_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevopenwith)

INSTALL_TARGETS(/lib/kde3 kdevopenwith)

INSTALL_FILES(/share/servicetypes FILES kdevopenwith.desktop)

#original Makefile.am contents follow:

## Here resides the openwith part.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevopenwith.la
#libkdevopenwith_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevopenwith_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevopenwith_la_SOURCES = openwithpart.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevopenwith.desktop
#


SET(kdevdoxygen_PART_SRCS
doxygenpart.cpp
doxygenconfigwidget.cpp
input.cpp
config.cpp
version.cpp
messages.cpp
)

KDE_AUTOMOC(${kdevdoxygen_PART_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/ ${CMAKE_SOURCE_DIR}/src/)

ADD_LIBRARY(kdevdoxygen MODULE ${kdevdoxygen_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevdoxygen ${KDEV_PART_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevdoxygen)

INSTALL_TARGETS(/lib/kde3 kdevdoxygen)

INSTALL_FILES(/share/servicetypes FILES kdevdoxygen.desktop)
INSTALL_FILES(/share/apps/kdevdoxygen FILES kdevdoxygen.rc)

#original Makefile.am contents follow:

## Here resides the doxygen part
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/src -DDOXYWIZARD \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = libkdevdoxygen.la
#libkdevdoxygen_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevdoxygen_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevdoxygen_la_SOURCES = doxygenpart.cpp doxygenconfigwidget.cpp input.cpp config.cpp version.cpp messages.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevdoxygen.desktop
#
#rcdir = $(kde_datadir)/kdevdoxygen
#rc_DATA = kdevdoxygen.rc

SUBDIRS( integrator )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevclearcase_PART_SRCS
clearcasepart.cpp
commentdlg.cpp
)

KDE_AUTOMOC(${kdevclearcase_PART_SRCS})

ADD_LIBRARY(kdevclearcase MODULE ${kdevclearcase_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevclearcase ${QT_AND_KDECORE_LIBS} kdeui kdevelop)

KDE_CREATE_LIBTOOL_FILE(kdevclearcase)

INSTALL_TARGETS(/lib/kde3 kdevclearcase)
INSTALL_FILES(/share/services FILES kdevclearcase.desktop )

#original Makefile.am contents follow:

## Here resides the clearcase part
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevclearcase.la
#libkdevclearcase_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevclearcase_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevclearcase_la_SOURCES = clearcasepart.cpp commentdlg.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevclearcase.desktop
#
#SUBDIRS = integrator
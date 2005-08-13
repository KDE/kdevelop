SET(kdevcopyto_PART_SRCS
copyto_part.cpp
copytodialog.cpp
)

KDE_AUTOMOC(${kdevcopyto_PART_SRCS})

SET( kdevcopyto_UI
copytodlgbase.ui
)

KDE_ADD_UI_FILES(kdevcopyto_PART_SRCS ${kdevcopyto_UI} )

ADD_LIBRARY(kdevcopyto MODULE ${kdevcopyto_PART_SRCS})

KDE_CREATE_LIBTOOL_FILE(kdevcopyto)

INSTALL_TARGETS(/lib/kde3 kdevcopyto)

INSTALL_FILES(/share/servicetypes FILES kdevcopyto.desktop)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevcopyto.la
#libkdevcopyto_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevcopyto_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevcopyto_la_SOURCES = copyto_part.cpp copytodlgbase.ui copytodialog.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevcopyto.desktop
#
#rcdir = $(kde_datadir)/kdevcopyto
#noinst_HEADERS = copytodialog.h


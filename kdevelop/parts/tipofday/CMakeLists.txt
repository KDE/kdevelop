SET(kdevtipofday_PART_SRCS
tipofday_part.cpp
)

KDE_AUTOMOC(${kdevtipofday_PART_SRCS})

ADD_LIBRARY(kdevtipofday MODULE ${kdevtipofday_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevtipofday ${KDEV_PART_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevtipofday)

INSTALL_TARGETS(/lib/kde3 kdevtipofday)

INSTALL_FILES(/share/servicetypes FILES kdevtipofday.desktop)
INSTALL_FILES(/share/apps/kdevtipofday FILES kdevpart_tipofday.rc)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevtipofday.la
#libkdevtipofday_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevtipofday_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevtipofday_la_SOURCES = tipofday_part.cpp
#
#METASOURCES = AUTO
#KDE_ICON = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevtipofday.desktop
#
#tip_DATA = tips
#tipdir = $(kde_datadir)/kdevtipofday
#
#rc_DATA = kdevpart_tipofday.rc
#rcdir = $(kde_datadir)/kdevtipofday
#
#messages: rc.cpp
#	$(PREPARETIPS) > tips.cc
#	$(XGETTEXT) *.cpp *.h tips.cc -o $(podir)/kdevtipofday.pot
#	rm -f tips.cc


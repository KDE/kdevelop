SUBDIRS( app_templates file_templates doc )

SET(kdevperlsupport_PART_SRCS
perlparser.cpp
perlsupportpart.cpp
perlconfigwidget.cpp
)

KDE_AUTOMOC(${kdevperlsupport_PART_SRCS})

SET(
perlconfigwidgetbase.ui
)

KDE_ADD_UI_FILES(kdevperlsupport_PART_SRCS ${kdevperlsupport_UI}

ADD_LIBRARY(kdevperlsupport SHARED ${kdevperlsupport_PART_SRCS})



#original Makefile.am contents follow:

## Here resides the Perl support part.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	$(all_includes)
#SUBDIRS = app_templates file_templates doc
#
#
#kde_module_LTLIBRARIES = libkdevperlsupport.la kio_perldoc.la
#libkdevperlsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevperlsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevperlsupport_la_SOURCES = perlparser.cpp perlsupportpart.cpp perlconfigwidget.cpp perlconfigwidgetbase.ui
#
#kio_perldoc_la_SOURCES = perldoc.cpp
#kio_perldoc_la_LIBADD = $(LIB_KIO)
#kio_perldoc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevperlsupport.desktop perldoc.protocol
#
#rcdir = $(kde_datadir)/kdevperlsupport
#rc_DATA = kdevperlsupport.rc

#######################
#previous CMakeLists.txt contents follow:


SET(kdevadaproject_PART_SRCS
adaproject_part.cpp
adaproject_widget.cpp
adaprojectoptionsdlg.cpp
adaglobaloptionsdlg.cpp
service.cpp
)

KDE_AUTOMOC(${kdevadaproject_PART_SRCS})

SET(
adaproject_optionsdlgbase.ui
)

KDE_ADD_UI_FILES(kdevadaproject_PART_SRCS ${kdevadaproject_UI}

ADD_LIBRARY(kdevadaproject SHARED ${kdevadaproject_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/buildtools/lib/base -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
#	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevadaproject.la
#libkdevadaproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevadaproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/buildtools/lib/base/libkdevbuildbase.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la
#
#libkdevadaproject_la_SOURCES = adaproject_part.cpp adaproject_widget.cpp adaproject_optionsdlgbase.ui adaprojectoptionsdlg.cpp adaglobaloptionsdlg.cpp service.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevadaproject.desktop
#
#rcdir = $(kde_datadir)/kdevadaproject
#rc_DATA = kdevadaproject.rc

#######################
#previous CMakeLists.txt contents follow:


SET(kdevpascalproject_PART_SRCS
pascalproject_part.cpp
pascalproject_widget.cpp
pascalprojectoptionsdlg.cpp
pascalglobaloptionsdlg.cpp
service.cpp
)

KDE_AUTOMOC(${kdevpascalproject_PART_SRCS})

SET(
pascalproject_optionsdlgbase.ui
)

KDE_ADD_UI_FILES(kdevpascalproject_PART_SRCS ${kdevpascalproject_UI}

ADD_LIBRARY(kdevpascalproject SHARED ${kdevpascalproject_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
#	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
#	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_builddir)/buildtools/lib/widgets \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = libkdevpascalproject.la
#libkdevpascalproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevpascalproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la $(top_builddir)/buildtools/lib/base/libkdevbuildbase.la
#
#libkdevpascalproject_la_SOURCES = pascalproject_part.cpp pascalproject_widget.cpp pascalproject_optionsdlgbase.ui pascalprojectoptionsdlg.cpp pascalglobaloptionsdlg.cpp service.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevpascalproject.desktop
#
#rcdir = $(kde_datadir)/kdevpascalproject
#rc_DATA = kdevpascalproject.rc
#
#
#
#

#######################
#previous CMakeLists.txt contents follow:


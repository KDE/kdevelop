SET(kdevhaskellproject_PART_SRCS
haskellproject_part.cpp
haskellprojectoptionsdlg.cpp
)

KDE_AUTOMOC(${kdevhaskellproject_PART_SRCS})

SET(
haskellproject_optionsdlgbase.ui
)

KDE_ADD_UI_FILES(kdevhaskellproject_PART_SRCS ${kdevhaskellproject_UI}

ADD_LIBRARY(kdevhaskellproject SHARED ${kdevhaskellproject_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
#	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
#	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_builddir)/buildtools/lib/widgets \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = libkdevhaskellproject.la
#libkdevhaskellproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevhaskellproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/buildtools/lib/base/libkdevbuildbase.la $(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la
#
#libkdevhaskellproject_la_SOURCES = haskellproject_part.cpp haskellproject_optionsdlgbase.ui haskellprojectoptionsdlg.cpp
## haskellproject_widget.cpp haskellproject_optionsdlgbase.ui haskellprojectoptionsdlg.cpp haskellglobaloptionsdlg.cpp service.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevhaskellproject.desktop
#
#rcdir = $(kde_datadir)/kdevhaskellproject
#rc_DATA = kdevhaskellproject.rc

#######################
#previous CMakeLists.txt contents follow:


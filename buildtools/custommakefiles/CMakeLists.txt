SET(kdevcustomproject_PART_SRCS
customprojectpart.cpp
custombuildoptionswidget.cpp
custommakeconfigwidget.cpp
)

KDE_AUTOMOC(${kdevcustomproject_PART_SRCS})

SET(
custombuildoptionswidgetbase.ui
custommakeconfigwidgetbase.ui
)

KDE_ADD_UI_FILES(kdevcustomproject_PART_SRCS ${kdevcustomproject_UI}

ADD_LIBRARY(kdevcustomproject SHARED ${kdevcustomproject_PART_SRCS})



#original Makefile.am contents follow:

## Here resides the custom project part.
#
#INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
#	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_builddir)/buildtools/lib/widgets \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = libkdevcustomproject.la
#libkdevcustomproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevcustomproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la $(top_builddir)/buildtools/lib/base/libkdevbuildbase.la
#
#libkdevcustomproject_la_SOURCES = customprojectpart.cpp custombuildoptionswidgetbase.ui custombuildoptionswidget.cpp custommakeconfigwidgetbase.ui custommakeconfigwidget.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevcustomproject.desktop
#
#rcdir = $(kde_datadir)/kdevcustomproject
#rc_DATA = kdevcustomproject.rc

#######################
#previous CMakeLists.txt contents follow:


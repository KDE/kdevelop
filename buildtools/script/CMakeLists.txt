INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/buildtools/lib/base ${CMAKE_SOURCE_DIR}/buildtools/lib/widgets ${CMAKE_BINARY_DIR}/buildtools/lib/widgets ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/external_interfaces )

SET(kdevscriptproject_PART_SRCS
scriptprojectpart.cpp
scriptoptionswidget.cpp
scriptnewfiledlg.cpp
)

SET( kdevscriptproject_UI
scriptoptionswidgetbase.ui
)

KDE_AUTOMOC(${kdevscriptproject_PART_SRCS})

KDE_ADD_UI_FILES(kdevscriptproject_PART_SRCS ${kdevscriptproject_UI} )

ADD_LIBRARY(kdevscriptproject MODULE ${kdevscriptproject_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevscriptproject ${KDEV_BUILDTOOLS_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevscriptproject)

INSTALL_TARGETS(/lib/kde3 kdevscriptproject)
INSTALL_FILES(/share/services FILES kdevscriptproject.desktop )
INSTALL_FILES(/share/apps/kdevscriptproject FILES kdevscriptproject.rc )


#original Makefile.am contents follow:

## Here resides the script project part.
## This is a generic part for various scripting languages that simply
## includes all files from the project directory without managing
## them. If a language needs more specific features, it deserves
## its own part (or the respective features should be added to the
## language support part)
#
#INCLUDES = -I$(top_srcdir)/buildtools/lib/base \
#	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/lib/external_interfaces $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevscriptproject.la
#libkdevscriptproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevscriptproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la $(top_builddir)/buildtools/lib/base/libkdevbuildbase.la
#
#libkdevscriptproject_la_SOURCES = scriptprojectpart.cpp scriptoptionswidget.cpp scriptoptionswidgetbase.ui scriptnewfiledlg.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevscriptproject.desktop
#
#rcdir = $(kde_datadir)/kdevscriptproject
#rc_DATA = kdevscriptproject.rc

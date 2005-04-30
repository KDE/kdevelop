INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/buildtools/lib/base ${CMAKE_SOURCE_DIR}/buildtools/lib/widgets ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/interfaces/extras ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevhaskellproject_PART_SRCS
haskellproject_part.cpp
haskellprojectoptionsdlg.cpp
)

SET( kdevhaskellproject_UI
haskellproject_optionsdlgbase.ui
)

KDE_AUTOMOC(${kdevhaskellproject_PART_SRCS})

KDE_ADD_UI_FILES(kdevhaskellproject_PART_SRCS ${kdevhaskellproject_UI} )

ADD_LIBRARY(kdevhaskellproject SHARED ${kdevhaskellproject_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevhaskellproject ${KDEV_BUILDTOOLS_LIBS} kparts)


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

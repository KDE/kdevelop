INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/buildtools/lib/base ${CMAKE_SOURCE_DIR}/buildtools/lib/widgets ${CMAKE_BINARY_DIR}/buildtools/lib/widgets ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/interfaces/extras ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevpascalproject_PART_SRCS
pascalproject_part.cpp
pascalproject_widget.cpp
pascalprojectoptionsdlg.cpp
pascalglobaloptionsdlg.cpp
service.cpp
)

SET( kdevpascalproject_UI
pascalproject_optionsdlgbase.ui
)

KDE_AUTOMOC(${kdevpascalproject_PART_SRCS})

KDE_ADD_UI_FILES(kdevpascalproject_PART_SRCS ${kdevpascalproject_UI} )

ADD_LIBRARY(kdevpascalproject MODULE ${kdevpascalproject_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevpascalproject ${KDEV_BUILDTOOLS_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevpascalproject)

INSTALL_TARGETS(/lib/kde3 kdevpascalproject)
INSTALL_FILES(/share/services FILES kdevpascalproject.desktop )
INSTALL_FILES(/share/apps/kdevpascalproject FILES kdevpascalproject.rc )

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

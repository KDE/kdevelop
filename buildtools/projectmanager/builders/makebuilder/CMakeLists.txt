INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/buildtools/lib/widgets ${CMAKE_BINARY_DIR}/buildtools/lib/widgets ${CMAKE_SOURCE_DIR}/buildtools/projectmanager/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR})

SET(kdevmakebuilder_LIB_SRCS
kdevmakebuilder.cpp
)

KDE_AUTOMOC(${kdevmakebuilder_LIB_SRCS})

KDE_ADD_MOC_FILES(kdevmakebuilder_LIB_SRCS  kdevmakebuilder.h)

ADD_LIBRARY(kdevmakebuilder SHARED ${kdevmakebuilder_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevmakebuilder ${QT_LIBRARIES} kdecore kdevelop projectmanager_interfaces kdevbuildtoolswidgets)

INSTALL_TARGETS(/lib kdevmakebuilder)

INSTALL_FILES(/share/services FILES kdevmakebuilder.desktop )

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/buildtools/lib/widgets \
#	-I$(top_srcdir)/buildtools/projectmanager/interfaces -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_builddir)/buildtools/lib/widgets \
#	$(all_includes)
#
#METASOURCES = AUTO
#
#lib_LTLIBRARIES = \
#    libkdevmakebuilder.la
#
#noinst_HEADERS = \
#    kdevmakebuilder.h
#
#libkdevmakebuilder_la_SOURCES = \
#    kdevmakebuilder.cpp
#
#kde_services_DATA =  kdevmakebuilder.desktop
#
#libkdevmakebuilder_la_LIBADD = \
#	$(top_builddir)/buildtools/projectmanager/interfaces/libprojectmanager_interfaces.la $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la
#
#libkdevmakebuilder_la_LDFLAGS = \
#    -module $(all_libraries) $(KDE_PLUGIN)

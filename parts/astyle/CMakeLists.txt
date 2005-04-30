SET(kdevastyle_PART_SRCS
astyle_part.cpp
astyle_widget.cpp
astyle_adaptor.cpp
)

KDE_AUTOMOC(${kdevastyle_PART_SRCS})

SET( kdevastyle_UI
astyleconfig.ui
)

KDE_ADD_UI_FILES(kdevastyle_PART_SRCS ${kdevastyle_UI} )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/astyle/ ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/)

ADD_LIBRARY(kdevastyle SHARED ${kdevastyle_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevastyle ${KDEV_PART_LIBS} astyle)


#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/astyle -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevastyle.la
#libkdevastyle_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevastyle_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/lib/astyle/libastyle.la
#
#libkdevastyle_la_SOURCES = astyle_part.cpp astyleconfig.ui astyle_widget.cpp astyle_adaptor.cpp
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevastyle.desktop
#
#rc_DATA = kdevpart_astyle.rc
#rcdir = $(kde_datadir)/kdevastyle

#######################
#previous CMakeLists.txt contents follow:

#SET(kdevastyle_PART_SRCS
#astyle_part.cpp
#astyle_widget.cpp
#astyle_adaptor.cpp
#)
#
#KDE_AUTOMOC(${kdevastyle_PART_SRCS})
#
#SET( kdevastyle_UI
#astyleconfig.ui
#)
#
#KDE_ADD_UI_FILES(kdevastyle_PART_SRCS ${kdevastyle_UI} )
#
#ADD_LIBRARY(kdevastyle SHARED ${kdevastyle_PART_SRCS})
#

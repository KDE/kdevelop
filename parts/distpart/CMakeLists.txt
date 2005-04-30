SET(kdevdistpart_PART_SRCS
distpart_part.cpp
distpart_widget.cpp
packagebase.cpp
lsmsupport.cpp
specsupport.cpp
)

SET( kdevdistpart_UI
distpart_ui.ui
)

KDE_AUTOMOC(${kdevdistpart_PART_SRCS})

KDE_ADD_UI_FILES(kdevdistpart_PART_SRCS ${kdevdistpart_UI} )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/extensions/)

ADD_LIBRARY(kdevdistpart SHARED ${kdevdistpart_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevdistpart ${KDEV_PART_LIBS})


#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevdistpart.la
#libkdevdistpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevdistpart_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevdistpart_la_SOURCES = distpart_ui.ui distpart_part.cpp distpart_widget.cpp packagebase.cpp lsmsupport.cpp specsupport.cpp
#
#METASOURCES = AUTO
#KDEICON = AUTO
#servicedir = $(kde_servicesdir)
#service_DATA = kdevdistpart.desktop
#
#rc_DATA = kdevpart_distpart.rc
#rcdir = $(kde_datadir)/kdevdistpart


SET(kdevvalgrind_PART_SRCS
valgrind_part.cpp
valgrind_widget.cpp
valgrind_dialog.cpp
valgrinditem.cpp
)

SET( kdevvalgrind_UI
dialog_widget.ui
)

KDE_AUTOMOC(${kdevvalgrind_PART_SRCS})

KDE_ADD_UI_FILES(kdevvalgrind_PART_SRCS ${kdevvalgrind_UI} )

ADD_LIBRARY(kdevvalgrind SHARED ${kdevvalgrind_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevvalgrind ${KDEV_PART_LIBS})


#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevvalgrind.la
#libkdevvalgrind_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevvalgrind_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#
#libkdevvalgrind_la_SOURCES = dialog_widget.ui valgrind_part.cpp valgrind_widget.cpp valgrind_dialog.cpp valgrinditem.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevvalgrind.desktop
#
#rcdir = $(kde_datadir)/kdevvalgrind
#rc_DATA = kdevpart_valgrind.rc

#######################
#previous CMakeLists.txt contents follow:

#SET(kdevvalgrind_PART_SRCS
#valgrind_part.cpp
#valgrind_widget.cpp
#valgrind_dialog.cpp
#valgrinditem.cpp
#)
#
#KDE_AUTOMOC(${kdevvalgrind_PART_SRCS})
#
#SET( kdevvalgrind_UI
#dialog_widget.ui
#)
#
#KDE_ADD_UI_FILES(kdevvalgrind_PART_SRCS ${kdevvalgrind_UI} )
#
#ADD_LIBRARY(kdevvalgrind SHARED ${kdevvalgrind_PART_SRCS})
#

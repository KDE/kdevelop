SET(kdevreplace_PART_SRCS
replace_part.cpp
replace_widget.cpp
replaceitem.cpp
replaceview.cpp
replacedlgimpl.cpp
)

SET( kdevreplace_UI
replacedlg.ui
)

KDE_AUTOMOC(${kdevreplace_PART_SRCS})

KDE_ADD_UI_FILES(kdevreplace_PART_SRCS ${kdevreplace_UI} )

ADD_LIBRARY(kdevreplace MODULE ${kdevreplace_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevreplace ${KDEV_PART_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevreplace)

INSTALL_TARGETS(/lib/kde3 kdevreplace)

INSTALL_FILES(/share/servicetypes FILES kdevreplace.desktop )
INSTALL_FILES(/share/apps/kdevreplace FILES kdevpart_replace.rc)

#original Makefile.am contents follow:

## Here resides the replace part (search and replace in the entire project)
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevreplace.la
#libkdevreplace_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevreplace_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#libkdevreplace_la_SOURCES = replace_part.cpp replace_widget.cpp replacedlg.ui replaceitem.cpp replaceview.cpp replacedlgimpl.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevreplace.desktop
#
#rcdir = $(kde_datadir)/kdevreplace
#rc_DATA = kdevpart_replace.rc


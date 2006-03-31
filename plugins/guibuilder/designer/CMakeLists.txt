INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${KDE4_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(kdevdesignerpart_PART_SRCS
kdevdesigner_part.cpp
)

KDE4_AUTOMOC(${kdevdesignerpart_PART_SRCS})

KDE4_ADD_PLUGIN(kdevdesignerpart WITH_PREFIX ${kdevdesignerpart_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevdesignerpart ${KDE4_KPARTS_LIBS} kdevinterfaces QtDesigner_debug QtDesignerComponents_debug)

INSTALL_TARGETS(/lib/kde4 kdevdesignerpart )


########### install files ###############

INSTALL_FILES( /share/services FILES  kdevdesigner_part.desktop )



#original Makefile.am contents follow:

## Here resides the designer part.
#
#INCLUDES = \
#  -I$(top_srcdir)/lib/interfaces \
#  -I$(top_srcdir)/lib/util \
#  $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevdesignerpart.la
#libkdevdesignerpart_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevdesignerpart_la_LIBADD = \
#  $(top_builddir)/lib/libkdevelop.la \
#  $(LIB_QTDESIGNER) $(LIB_KFILE) $(LIB_KPARTS)
#
#libkdevdesignerpart_la_SOURCES = \
#  kdevdesigner_part.cpp
#
#METASOURCES = AUTO
#
##partrcdir = $(kde_datadir)/kdevdesignerpart
##partrc_DATA = kdevdesigner_part.rc
#
#partdesktopdir = $(kde_servicesdir)
#partdesktop_DATA = kdevdesigner_part.desktop

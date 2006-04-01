ADD_SUBDIRECTORY( designer )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${KDE4_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

SET(kdevguibuilder_PART_SRCS
guibuilder_part.cpp
formwindow_page.cpp
)

kde4_automoc(${kdevguibuilder_PART_SRCS})

kde4_add_plugin(kdevguibuilder ${kdevguibuilder_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevguibuilder  ${KDE4_KDECORE_LIBS} kdevinterfaces QtDesigner_debug QtDesignerComponents_debug)

INSTALL_TARGETS(/lib/kde4 kdevguibuilder )
kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kdevguibuilder )

########### install files ###############

INSTALL_FILES( /share/services FILES  kdevguibuilder.desktop )



#original Makefile.am contents follow:

## Here resides the guibuilder part.
#
#SUBDIRS = . designer
#
#INCLUDES = \
#  -I$(top_srcdir)/lib/interfaces \
#  -I$(top_srcdir)/lib/util \
#  $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevguibuilder.la
#libkdevguibuilder_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevguibuilder_la_LIBADD = \
#  $(top_builddir)/lib/libkdevelop.la \
#  $(LIB_QTDESIGNER_COMPONENTS) $(LIB_QTDESIGNER)
#
#libkdevguibuilder_la_SOURCES = \
#  guibuilder_part.cpp \
#  formwindow_page.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevguibuilder.desktop
#

SET(kdevquickopen_PART_SRCS
quickopen_part.cpp
quickopendialog.cpp
quickopenclassdialog.cpp
quickopenfiledialog.cpp
quickopenfunctiondialog.cpp
quickopenfunctionchooseform.cpp
)

KDE_AUTOMOC(${kdevquickopen_PART_SRCS})

SET( kdevquickopen_UI
quickopenbase.ui
quickopenfunctionchooseformbase.ui
)

KDE_ADD_UI_FILES(kdevquickopen_PART_SRCS ${kdevquickopen_UI} )

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/external/ ${CMAKE_SOURCE_DIR}/lib/widgets/)

ADD_LIBRARY(kdevquickopen SHARED ${kdevquickopen_PART_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevquickopen.la
#libkdevquickopen_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevquickopen_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libkdevwidgets.la $(LIB_KHTML)
#
#libkdevquickopen_la_SOURCES = quickopen_part.cpp quickopendialog.cpp quickopenclassdialog.cpp quickopenbase.ui quickopenfiledialog.cpp quickopenfunctiondialog.cpp quickopenfunctionchooseformbase.ui quickopenfunctionchooseform.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevquickopen.desktop
#
#rcdir = $(kde_datadir)/kdevquickopen
#rc_DATA = kdevpart_quickopen.rc
#noinst_HEADERS = quickopendialog.h quickopenclassdialog.h quickopenfiledialog.h quickopenfunctiondialog.h quickopenfunctionchooseform.h

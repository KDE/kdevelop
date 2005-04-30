INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/buildtools/projectmanager/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/project ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevautomakeimporter_LIB_SRCS
kdevautomakeimporter.cpp
automakeprojectmodel.cpp
)

KDE_AUTOMOC(${kdevautomakeimporter_LIB_SRCS})

ADD_LIBRARY(kdevautomakeimporter SHARED ${kdevautomakeimporter_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevautomakeimporter ${QT_LIBRARIES} kdecore ${CMAKE_BINARY_DIR}/lib/libkdevelop.so projectmanager_interfaces)

#original Makefile.am contents follow:

#INCLUDES = \
#    -I$(top_srcdir)/buildtools/projectmanager/interfaces \
#    -I$(top_srcdir)/lib/interfaces \
#    -I$(top_srcdir)/lib/project \
#    -I$(top_srcdir)/lib/util \
#    $(all_includes)
#
#METASOURCES = AUTO
#
#lib_LTLIBRARIES = \
#    libkdevautomakeimporter.la
#
#noinst_HEADERS = \
#    kdevautomakeimporter.h \
#    automakeprojectmodel.h
#
#libkdevautomakeimporter_la_SOURCES = \
#    kdevautomakeimporter.cpp \
#    automakeprojectmodel.cpp
#
#kde_services_DATA =  kdevautomakeimporter.desktop
#
#libkdevautomakeimporter_la_LIBADD = \
#    $(top_builddir)/buildtools/projectmanager/interfaces/libprojectmanager_interfaces.la \
#    $(top_builddir)/lib/libkdevelop.la
#
#libkdevautomakeimporter_la_LDFLAGS = \
#    -module $(all_libraries) $(KDE_PLUGIN)

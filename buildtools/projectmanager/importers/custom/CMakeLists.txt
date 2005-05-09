INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/buildtools/projectmanager/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/project ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevcustomimporter_LIB_SRCS
kdevcustomimporter.cpp
)

KDE_AUTOMOC(${kdevcustomimporter_LIB_SRCS})

ADD_LIBRARY(kdevcustomimporter SHARED ${kdevcustomimporter_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevcustomimporter ${QT_LIBRARIES} kdecore kdevelop projectmanager_interfaces)


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
#    libkdevcustomimporter.la
#
#noinst_HEADERS = \
#    kdevcustomimporter.h
#
#libkdevcustomimporter_la_SOURCES = \
#    kdevcustomimporter.cpp
#
#kde_services_DATA =  kdevcustomimporter.desktop
#
#libkdevcustomimporter_la_LIBADD = \
#    $(top_builddir)/buildtools/projectmanager/interfaces/libprojectmanager_interfaces.la \
#    $(top_builddir)/lib/libkdevelop.la
#
#libkdevcustomimporter_la_LDFLAGS = \
#    -module $(all_libraries) $(KDE_PLUGIN)

SET(qmakeimporter_LIB_SRCS
kdevqmakeeditor.cpp
qmakeprojectmodel.cpp
modelcreator.cpp
)

KDE_AUTOMOC(${qmakeimporter_LIB_SRCS})

ADD_LIBRARY(qmakeimporter SHARED ${qmakeimporter_LIB_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/buildtools/lib/parsers/qmake \
#	-I$(top_srcdir)/buildtools/projectmanager/interfaces -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES = libqmakeimporter.la
#libqmakeimporter_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
#noinst_HEADERS = kdevqmakeeditor.h qmakeprojectmodel.h modelcreator.h
#libqmakeimporter_la_SOURCES = kdevqmakeeditor.cpp qmakeprojectmodel.cpp \
#	modelcreator.cpp
#kde_services_DATA = kdevqmakeimporter.desktop
#libqmakeimporter_la_LIBADD = \
#	$(top_builddir)/buildtools/projectmanager/interfaces/libprojectmanager_interfaces.la $(top_builddir)/buildtools/lib/parsers/qmake/libkdevqmakeparser.la

#######################
#previous CMakeLists.txt contents follow:


SET(dockdevtocplugin_LIB_SRCS
dockdevtocplugin.cpp
)

KDE_AUTOMOC(${dockdevtocplugin_LIB_SRCS})

ADD_LIBRARY(dockdevtocplugin SHARED ${dockdevtocplugin_LIB_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/parts/documentation/interfaces $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES =  libdockdevtocplugin.la
#libdockdevtocplugin_la_LDFLAGS = $(all_libraries)
#kde_services_DATA =  dockdevtocplugin.desktop
#noinst_HEADERS =  dockdevtocplugin.h
#libdockdevtocplugin_la_SOURCES =  dockdevtocplugin.cpp
#libdockdevtocplugin_la_LIBADD =\
#	$(top_builddir)/parts/documentation/interfaces/libdocumentation_interfaces.la

#######################
#previous CMakeLists.txt contents follow:

#SET(dockdevtocplugin_LIB_SRCS
#dockdevtocplugin.cpp
#)
#
#KDE_AUTOMOC(${dockdevtocplugin_LIB_SRCS})
#
#ADD_LIBRARY(dockdevtocplugin SHARED ${dockdevtocplugin_LIB_SRCS})
#

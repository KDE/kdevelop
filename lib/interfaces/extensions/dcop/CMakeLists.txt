SET(kdevdcopextensions_STAT_SRCS
KDevAppFrontendIface.cpp
KDevMakeFrontendIface.cpp
KDevAppFrontendIface.skel
KDevMakeFrontendIface.skel
)

KDE_AUTOMOC(${kdevdcopextensions_STAT_SRCS})

ADD_LIBRARY(kdevdcopextensions STATIC ${kdevdcopextensions_STAT_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions $(all_includes)
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES = libkdevdcopextensions.la
#libkdevdcopextensions_la_LDFLAGS = $(all_libraries)
#libkdevdcopextensions_la_SOURCES = KDevAppFrontendIface.cpp \
#		KDevMakeFrontendIface.cpp KDevAppFrontendIface.skel KDevMakeFrontendIface.skel 
#
#dcopincludeextdir = $(includedir)/kdevelop/interfaces/extensions/dcop
#dcopincludeext_HEADERS = KDevAppFrontendIface.h KDevMakeFrontendIface.h

#######################
#previous CMakeLists.txt contents follow:

#
#SET(kdevdcopextensions_SRCS KDevAppFrontendIface.cpp KDevMakeFrontendIface.cpp )
#
#
#SET(kdevdcopextensions_DCOP_SRCS KDevAppFrontendIface.h KDevMakeFrontendIface.h )
#
#KDE_ADD_DCOP_SKELS(kdevdcopextensions_SRCS ${kdevdcopextensions_DCOP_SRCS})
#
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..)
#
#ADD_LIBRARY(kdevdcopextensions ${kdevdcopextensions_SRCS})
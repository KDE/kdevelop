SET(kdevqui_STAT_SRCS
database.cpp
qwidgetfactory.cpp
)

KDE_AUTOMOC(${kdevqui_STAT_SRCS})

ADD_LIBRARY(kdevqui STATIC ${kdevqui_STAT_SRCS})



#original Makefile.am contents follow:

#KDE_CXXFLAGS=-UQT_NO_ASCII_CAST
#
#INCLUDES = -I$(top_srcdir)/kdevdesigner/shared $(all_includes)
#METASOURCES = AUTO
#
#noinst_LTLIBRARIES =  libkdevqui.la
#libkdevqui_la_SOURCES =  database.cpp qwidgetfactory.cpp
#
#libkdevqui_la_LDFLAGS = $(all_libraries)

#######################
#previous CMakeLists.txt contents follow:


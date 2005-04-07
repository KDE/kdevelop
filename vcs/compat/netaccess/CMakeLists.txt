SET(netaccess_STAT_SRCS
netaccess.cpp
)

KDE_AUTOMOC(${netaccess_STAT_SRCS})

ADD_LIBRARY(netaccess STATIC ${netaccess_STAT_SRCS})



#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#noinst_LTLIBRARIES = libnetaccess.la
#libnetaccess_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) $(LIB_KIO)
#libnetaccess_la_SOURCES = netaccess.cpp
#METASOURCES = AUTO

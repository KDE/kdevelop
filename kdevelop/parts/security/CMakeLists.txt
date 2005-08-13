SET(kdevsecurity_PART_SRCS
securitypart.cpp
securitywidget.cpp
securitychecker.cpp
)

KDE_AUTOMOC(${kdevsecurity_PART_SRCS})

ADD_LIBRARY(kdevsecurity MODULE ${kdevsecurity_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevsecurity ${KDEV_PART_LIBS})

KDE_CREATE_LIBTOOL_FILE(kdevsecurity)

INSTALL_TARGETS(/lib/kde3 kdevsecurity)

INSTALL_FILES(/share/servicetypes FILES kdevsecurity.desktop )

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util \
#	$(all_includes)
#METASOURCES = AUTO
#kde_module_LTLIBRARIES = libkdevsecurity.la
#libkdevsecurity_la_LDFLAGS = $(all_libraries)
#libkdevsecurity_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
#noinst_HEADERS = securitypart.h securitywidget.h securitychecker.h
#libkdevsecurity_la_SOURCES = securitypart.cpp securitywidget.cpp \
#	securitychecker.cpp
#servicedir = $(kde_servicesdir)
#service_DATA = kdevsecurity.desktop
#securitydatadir = $(kde_datadir)/kdevsecurity
#securitydata_DATA = patterns

#######################
#previous CMakeLists.txt contents follow:

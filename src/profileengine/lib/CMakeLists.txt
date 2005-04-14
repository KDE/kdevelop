INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/external_interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets ${CMAKE_SOURCE_DIR}/lib/structure )

SET(profileengine_LIB_SRCS
profileengine.cpp
profile.cpp
)

KDE_AUTOMOC(${profileengine_LIB_SRCS})

ADD_LIBRARY(profileengine SHARED ${profileengine_LIB_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/external_interfaces \
#	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets \
#	$(KDEVMDI_INCLUDES) -I$(top_srcdir)/lib/structure $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES = libprofileengine.la
#libprofileengine_la_LDFLAGS = $(all_libraries)
#libprofileengine_la_SOURCES = profileengine.cpp profile.cpp
#
#profileincludedir = $(includedir)/kdevelop/shell/profileengine
#profileinclude_HEADERS = profileengine.h profile.h
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevutil kdevinterfaces
#DOXYGEN_PROJECTNAME = KDevelop Shell Profiles Library
#DOXYGEN_DOCDIRPREFIX = kdevprofiles
#include ../../../Doxyfile.am

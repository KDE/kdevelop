INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/external_interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets ${CMAKE_SOURCE_DIR}/src/profileengine/lib ${CMAKE_SOURCE_DIR}/lib/structure )

SET(profileeditor_STAT_SRCS
profileeditor.cpp
)

KDE_AUTOMOC(${profileeditor_STAT_SRCS})

SET( profileeditor_UI
profileeditorbase.ui
addprofilewidget.ui
)

KDE_ADD_UI_FILES(profileeditor_STAT_SRCS ${profileeditor_UI} )

ADD_LIBRARY(profileeditor STATIC ${profileeditor_STAT_SRCS})

SET(kdevprofileeditor_SRCS
main.cpp
)

KDE_AUTOMOC(${kdevprofileeditor_SRCS})

ADD_EXECUTABLE(kdevprofileeditor ${kdevprofileeditor_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/external_interfaces \
#	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets \
#	-I$(top_srcdir)/src/profileengine/lib $(KDEVMDI_INCLUDES) -I$(top_srcdir)/lib/structure $(all_includes)
#METASOURCES = AUTO
#
#libprofileeditor_la_LDFLAGS = $(all_libraries)
#libprofileeditor_la_LIBADD = $(top_builddir)/src/profileengine/lib/libprofileengine.la
#noinst_LTLIBRARIES = libprofileeditor.la
#libprofileeditor_la_SOURCES = profileeditorbase.ui profileeditor.cpp \
#	addprofilewidget.ui
#noinst_HEADERS = profileeditor.h
#
#
#bin_PROGRAMS = kdevprofileeditor
#kdevprofileeditor_LDFLAGS = $(all_libraries)
#kdevprofileeditor_LDADD = \
#	$(top_builddir)/src/profileengine/lib/libprofileengine.la libprofileeditor.la \
#	$(LIB_KDEUI) $(LIB_KIO)
#kdevprofileeditor_SOURCES = main.cpp
#
#

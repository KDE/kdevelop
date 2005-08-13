INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util )

SET(designerintegration_LIB_SRCS
implementationwidget.cpp
qtdesignerintegration.cpp
)

SET( designerintegration_UI
implementationwidgetbase.ui
)

KDE_AUTOMOC(${designerintegration_LIB_SRCS})

KDE_ADD_UI_FILES(designerintegration_LIB_SRCS ${designerintegration_UI} )

ADD_LIBRARY(designerintegration SHARED ${designerintegration_LIB_SRCS})

TARGET_LINK_LIBRARIES(designerintegration ${QT_AND_KDECORE_LIBS} kdeui kdevelop)

INSTALL_TARGETS(/lib designerintegration)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
#	$(all_includes)
#METASOURCES = AUTO
#libdesignerintegration_la_LDFLAGS = $(all_libraries)
#lib_LTLIBRARIES = libdesignerintegration.la
#libdesignerintegration_la_SOURCES = implementationwidgetbase.ui \
#	implementationwidget.cpp qtdesignerintegration.cpp
#
#langincludedirdir = $(includedir)/kdevelop/languages/designer_integration
#langincludedir_HEADERS = qtdesignerintegration.h implementationwidget.h implementationwidgetbase.h
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevinterfaces kdevutil
#DOXYGEN_PROJECTNAME = KDevelop Designer Integration Support Library
#include ../../../Doxyfile.am

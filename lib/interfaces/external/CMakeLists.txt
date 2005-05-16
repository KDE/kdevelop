SET(kinterfacedesigner_LIB_SRCS
designer.cpp
)

KDE_AUTOMOC(${kinterfacedesigner_LIB_SRCS})

ADD_LIBRARY(kinterfacedesigner SHARED ${kinterfacedesigner_LIB_SRCS})

TARGET_LINK_LIBRARIES(kinterfacedesigner ${QT_LIBRARIES} kdecore kdeui kparts)

INSTALL_TARGETS(/lib kinterfacedesigner)

INSTALL_FILES(/include/kinterfacedesigner FILES designer.h)

#original Makefile.am contents follow:

#INCLUDES = $(all_includes)
#
#METASOURCES = AUTO
#
#kinterfacedesignerdir = $(includedir)/kinterfacedesigner
#kinterfacedesigner_HEADERS = designer.h
#
#lib_LTLIBRARIES =  libkinterfacedesigner.la
#libkinterfacedesigner_la_LDFLAGS = $(all_libraries)
#libkinterfacedesigner_la_SOURCES =  designer.cpp
#
#DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils
#DOXYGEN_PROJECTNAME = KInterfaceDesigner Library
#include ../../../Doxyfile.am

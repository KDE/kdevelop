INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/interfaces/extras ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets ${CMAKE_SOURCE_DIR}/src/profileengine/lib )

SET(d_LIB_SRCS
dmainwindow.cpp
buttonbar.cpp
button.cpp
ddockwindow.cpp
docksplitter.cpp
dtabwidget.cpp
)

KDE_AUTOMOC(${d_LIB_SRCS})

ADD_LIBRARY(d SHARED ${d_LIB_SRCS})

TARGET_LINK_LIBRARIES(d ${QT_AND_KDECORE_LIBS} kdeui kio kparts)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
#	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets \
#	-I$(top_srcdir)/src/profileengine/lib $(all_includes)
#METASOURCES = AUTO
#
#
#lib_LTLIBRARIES = libd.la
#libd_la_LDFLAGS = $(all_libraries)
#
#noinst_HEADERS = dmainwindow.h ddockwindow.h buttonbar.h  button.h  comdefs.h docksplitter.h dtabwidget.h
#libd_la_SOURCES = dmainwindow.cpp buttonbar.cpp \
#	button.cpp ddockwindow.cpp docksplitter.cpp dtabwidget.cpp
#libd_la_LIBADD = $(LIB_KPARTS)

SET(kdevclassview_PART_SRCS
classviewpart.cpp
classviewwidget.cpp
digraphview.cpp
hierarchydlg.cpp
navigator.cpp
viewcombos.cpp
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/lib/interfaces/external/ ${CMAKE_SOURCE_DIR}/lib/widgets/)

KDE_AUTOMOC(${kdevclassview_PART_SRCS})

ADD_LIBRARY(kdevclassview SHARED ${kdevclassview_PART_SRCS})



#original Makefile.am contents follow:

## Here resides the class view part.
#
#INCLUDES = -I$(top_srcdir)/lib/compat -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevclassview.la
#libkdevclassview_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevclassview_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libkdevwidgets.la $(LIB_KHTML)
#
#libkdevclassview_la_SOURCES = classviewpart.cpp classviewwidget.cpp \
#	digraphview.cpp hierarchydlg.cpp navigator.cpp viewcombos.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevclassview.desktop
#
#rcdir = $(kde_datadir)/kdevclassview
#rc_DATA = kdevclassview.rc
#
#
#QTDIR = /home/bernd/kdesrc/qt-copy
#
#foo: digraphview.cpp digraphview.h
#	$(QTDIR)/bin/moc digraphview.h > digraphview.moc ; \
#	g++ digraphview.cpp -I$(QTDIR)/include -L$(QTDIR)/lib -L/usr/X11R6/lib -lqt -lX11 -lXext -o foo
#noinst_HEADERS = viewcombos.h navigator.h 

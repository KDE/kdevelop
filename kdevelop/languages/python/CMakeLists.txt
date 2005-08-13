SUBDIRS( app_templates file_templates doc )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/languages/lib/designer_integration ${CMAKE_BINARY_DIR}/languages/lib/designer_integration ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/external ${CMAKE_SOURCE_DIR}/lib/util )

SET(kdevpythonsupport_PART_SRCS
pythonsupportpart.cpp
pythonconfigwidget.cpp
pythonimplementationwidget.cpp
qtdesignerpythonintegration.cpp
)

SET( kdevpythonsupport_UI
pythonconfigwidgetbase.ui
)

KDE_AUTOMOC(${kdevpythonsupport_PART_SRCS})

KDE_ADD_UI_FILES(kdevpythonsupport_PART_SRCS ${kdevpythonsupport_UI} )

ADD_LIBRARY(kdevpythonsupport SHARED ${kdevpythonsupport_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevpythonsupport kdevelop khtml kdevwidgets designerintegration)

KDE_CREATE_LIBTOOL_FILE(kdevpythonsupport)

INSTALL_TARGETS(/lib/kde3 kdevpythonsupport)

#original Makefile.am contents follow:

## Here resides the Python support part.
## Do not confuse this with the python part responsible for KDevelop scripting
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/languages/lib/designer_integration \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util -I$(top_builddir)/languages/lib/designer_integration \
#	$(all_includes)
#SUBDIRS = app_templates file_templates doc
#
#
#kde_module_LTLIBRARIES = libkdevpythonsupport.la kio_pydoc.la
#
#libkdevpythonsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevpythonsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/languages/lib/designer_integration/libdesignerintegration.la
#libkdevpythonsupport_la_SOURCES = pythonsupportpart.cpp pythonconfigwidget.cpp pythonconfigwidgetbase.ui \
#	pythonimplementationwidget.cpp qtdesignerpythonintegration.cpp
#
#kio_pydoc_la_SOURCES = pydoc.cpp
#kio_pydoc_la_LIBADD = $(LIB_KIO)
#kio_pydoc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#METASOURCES = AUTO
#
#slavedir = $(kde_datadir)/kio_pydoc
#slave_DATA = kde_pydoc.py kde_inspect.py
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevpythonsupport.desktop pydoc.protocol
#
#rcdir = $(kde_datadir)/kdevpythonsupport
#rc_DATA = kdevpythonsupport.rc

SUBDIRS = buildsystem

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/external_interfaces -I$(top_srcdir)/lib/project -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets/propeditor -I$(top_builddir)/lib/project $(all_includes)
METASOURCES = AUTO

lib_LTLIBRARIES =  libkdevgenericproject.la
libkdevgenericproject_la_SOURCES = detailslistview.cpp overviewlistview.cpp genericlistviewitem.cpp \
	genericproject_part.cpp genericproject_widget.cpp variantserializer.cpp
libkdevgenericproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
libkdevgenericproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevgenericproject_la_COMPILE_FIRST = $(top_builddir)/lib/project/removesubprojectdlgbase.h

noinst_HEADERS = detailslistview.h overviewlistview.h genericlistviewitem.h genericproject_part.h genericproject_widget.h variantserializer.h

servicedir = $(kde_servicesdir)
service_DATA = kdevgenericproject.desktop

rcdir = $(kde_datadir)/kdevgenericproject
rc_DATA = kdevgenericproject.rc
noinst_DATA =  kdevxmlproject.dtd

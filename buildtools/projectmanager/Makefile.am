INCLUDES = \
    -I$(top_srcdir)/buildtools/projectmanager/interfaces \
    -I$(top_srcdir)/lib/interfaces \
    -I$(top_srcdir)/lib/project \
    -I$(top_srcdir)/lib/util \
    -I$(top_srcdir)/lib/widgets/propeditor \
    $(all_includes)

SUBDIRS = interfaces . importers builders 

kde_module_LTLIBRARIES = libkdevprojectmanager.la
libkdevprojectmanager_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevprojectmanager_la_LIBADD = \
    $(top_builddir)/buildtools/projectmanager/interfaces/libprojectmanager_interfaces.la \
    $(top_builddir)/lib/widgets/propeditor/libkdevpropertyeditor.la \
    $(top_builddir)/lib/libkdevelop.la

libkdevprojectmanager_la_SOURCES = \
    kdevprojectmanager_part.cpp \
    kdevprojectmanager_widget.cpp \
    importprojectjob.cpp
    
METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevprojectmanager.desktop

rcdir = $(kde_datadir)/kdevprojectmanager
rc_DATA = \
    kdevprojectmanager.rc



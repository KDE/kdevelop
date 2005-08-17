INCLUDES = \
    -I$(top_srcdir)/lib/projectmanager/interfaces \
    -I$(top_srcdir)/lib/interfaces \
    -I$(top_srcdir)/lib/project \
    -I$(top_srcdir)/lib/util \
    $(all_includes)
    
METASOURCES = AUTO

lib_LTLIBRARIES = \
    libkdevgenericimporter.la

noinst_HEADERS = \
    kdevgenericimporter.h
    
libkdevgenericimporter_la_SOURCES = \
    kdevgenericimporter.cpp

kde_services_DATA = kdevgenericimporter.desktop

libkdevgenericimporter_la_LIBADD = \
    $(top_builddir)/lib/projectmanager/interfaces/libprojectmanager_interfaces.la \
    $(top_builddir)/lib/libkdevelop.la
    
libkdevgenericimporter_la_LDFLAGS = \
    -module $(all_libraries) $(KDE_PLUGIN)

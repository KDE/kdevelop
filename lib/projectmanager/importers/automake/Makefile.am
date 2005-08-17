INCLUDES = \
    -I$(top_srcdir)/buildtools/projectmanager/interfaces \
    -I$(top_srcdir)/lib/interfaces \
    -I$(top_srcdir)/lib/project \
    -I$(top_srcdir)/lib/util \
    $(all_includes)
    
METASOURCES = AUTO

lib_LTLIBRARIES = \
    libkdevautomakeimporter.la

noinst_HEADERS = \
    kdevautomakeimporter.h \
    automakeprojectmodel.h
    
libkdevautomakeimporter_la_SOURCES = \
    kdevautomakeimporter.cpp \
    automakeprojectmodel.cpp

kde_services_DATA =  kdevautomakeimporter.desktop

libkdevautomakeimporter_la_LIBADD = \
    $(top_builddir)/buildtools/projectmanager/interfaces/libprojectmanager_interfaces.la \
    $(top_builddir)/lib/libkdevelop.la
    
libkdevautomakeimporter_la_LDFLAGS = \
    -module $(all_libraries) $(KDE_PLUGIN)

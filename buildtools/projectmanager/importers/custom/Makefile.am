INCLUDES = \
    -I$(top_srcdir)/buildtools/projectmanager/interfaces \
    -I$(top_srcdir)/lib/interfaces \
    -I$(top_srcdir)/lib/project \
    -I$(top_srcdir)/lib/util \
    $(all_includes)
    
METASOURCES = AUTO

kde_module_LTLIBRARIES = \
    libkdevcustomimporter.la

noinst_HEADERS = \
    kdevcustomimporter.h
    
libkdevcustomimporter_la_SOURCES = \
    kdevcustomimporter.cpp

kde_services_DATA =  kdevcustomimporter.desktop

libkdevcustomimporter_la_LIBADD = \
    $(top_builddir)/buildtools/projectmanager/interfaces/libprojectmanager_interfaces.la \
    $(top_builddir)/lib/libkdevelop.la
    
libkdevcustomimporter_la_LDFLAGS = \
    -module $(all_libraries) $(KDE_PLUGIN)

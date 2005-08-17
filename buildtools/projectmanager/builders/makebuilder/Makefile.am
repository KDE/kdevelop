INCLUDES = -I$(top_srcdir)/lib/projectmanager/interfaces -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util \
        $(all_includes)
    
METASOURCES = AUTO

lib_LTLIBRARIES = \
    libkdevmakebuilder.la

noinst_HEADERS = \
    kdevmakebuilder.h
    
libkdevmakebuilder_la_SOURCES = \
    kdevmakebuilder.cpp

kde_services_DATA =  kdevmakebuilder.desktop

libkdevmakebuilder_la_LIBADD = \
	$(top_builddir)/lib/projectmanager/interfaces/libprojectmanager_interfaces.la $(top_builddir)/lib/libkdevelop.la

libkdevmakebuilder_la_LDFLAGS = \
    -module $(all_libraries) $(KDE_PLUGIN)

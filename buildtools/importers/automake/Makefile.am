INCLUDES = \
    -I$(top_srcdir)/lib/projectmanager/interfaces \
    -I$(top_srcdir)/lib/interfaces \
    -I$(top_srcdir)/lib/project \
    -I$(top_srcdir)/lib/util \
    $(all_includes)
    
METASOURCES = AUTO

lib_LTLIBRARIES = \
    libkdevautomakeimporter.la

libkdevautomakeimporter_la_SOURCES = autotoolsast.cpp autotoolsdriver.cpp \
	automakeprojectmodel.cpp autotools_yacc.cpp automakeimporter.cpp makefileinterface.cpp

kde_services_DATA =  kdevautomakeimporter.desktop

libkdevautomakeimporter_la_LIBADD = \
    $(top_builddir)/lib/projectmanager/libkdevprojectmanager.la \
    $(top_builddir)/lib/libkdevelop.la
    
libkdevautomakeimporter_la_LDFLAGS = \
    $(all_libraries) $(KDE_PLUGIN)

parser:
	cd $(srcdir) ; \
        bison -d autotools.yy -o autotools_yacc.cpp ; \
        mv -f autotools_yacc.hpp autotools_yacc.h ; \
        flex -oautotools_lex.cpp autotools.ll

EXTRA_DIST = autotools.yy autotools.ll

noinst_HEADERS = makefileinterface.h

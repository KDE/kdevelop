INCLUDES = $(all_includes) -I$(top_srcdir)/lib/interfaces

METASOURCES = AUTO

bin_PROGRAMS = editortest

editortest_SOURCES = main.cpp editortest.cpp
editortest_LDFLAGS = $(KDE_RPATH) $(all_libraries)
editortest_LDADD   = $(LIB_KPARTS) $(top_builddir)/lib/interfaces/keditor/libkeditorinterface.la

shellrcdir   = $(kde_datadir)/editortest
shellrc_DATA = editortest_shell.rc


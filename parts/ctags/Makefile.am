# Here resides the ctags part

INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)

#bin_PROGRAMS = test
#test_SOURCES = ctagscollection.cpp
#test_LDFLAGS = $(all_libraries)
#test_LDADD = $(LIB_KDEUI)

kde_module_LTLIBRARIES = libkdevctags.la
libkdevctags_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevctags_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevctags_la_SOURCES = ctagsfactory.cpp \
			  ctagspart.cpp \
			  ctagsdlg.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevctags.desktop

rcdir = $(kde_datadir)/kdevctags
rc_DATA = kdevctags.rc

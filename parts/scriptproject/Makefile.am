# Here resides the script project part.
# This is a generic part for various scripting languages that simply 
# displays all files related to that language in a tree. File operations
# may be implemented in the future. If a language needs more specific
# features, it deserves its own part (or the respective features should
# be added to the language support part)

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

lib_LTLIBRARIES = libkdevscriptproject.la
libkdevscriptproject_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevscriptproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevscriptproject_la_SOURCES = scriptprojectfactory.cpp \
				  scriptprojectpart.cpp \
				  scriptprojectwidget.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevscriptproject.desktop

#rcdir = $(kde_datadir)/kdevscriptproject
#rc_DATA = kdevscriptproject.rc

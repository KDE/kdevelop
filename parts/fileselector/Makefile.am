# Here resides the file selector part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo $(all_includes)

kde_module_LTLIBRARIES = libkdevfileselector.la
libkdevfileselector_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevfileselector_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevfileselector_la_SOURCES = fileselector_part.cpp \
                                  fileselectorwidget.cpp \
								  kfiledndiconview.cpp \
								  kfilednddetailview.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_fileselector.desktop

# not used currently
#rcdir = $(kde_datadir)/kdevfileselector
#rc_DATA = kdevfileselector.rc

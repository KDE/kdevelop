# Here resides the file selector part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/sourceinfo $(all_includes)

kde_module_LTLIBRARIES = libkdevfileselector.la
libkdevfileselector_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevfileselector_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevfileselector_la_SOURCES = fileselector_part.cpp fileselector_widget.cpp kactionselector.cpp kbookmarkhandler.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevfileselector.desktop

# not used currently
#rcdir = $(kde_datadir)/kdevfileselector
#rc_DATA = kdevfileselector.rc

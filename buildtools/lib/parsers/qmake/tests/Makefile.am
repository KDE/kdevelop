# This directory collects some classes related to
# project management for the sole purpose that they
# can be shared between parts.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
        -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets/propeditor \
        -I$(top_srcdir)/buildtools/lib/parsers/qmake \
        -I$(top_builddir)/buildtools/lib/parsers/qmake/tests \
        $(all_includes)

METASOURCES = AUTO

noinst_PROGRAMS = runner viewer

runner_LDFLAGS = $(all_libraries) $(LIB_KDECORE) $(KDE_RPATH)
runner_LDADD = $(top_builddir)/buildtools/lib/parsers/qmake/libkdevqmakeparser.la
runner_SOURCES = runner.cpp

viewer_LDFLAGS = $(all_libraries) $(LIB_KDECORE) $(KDE_RPATH)
viewer_LDADD = $(top_builddir)/buildtools/lib/parsers/qmake/libkdevqmakeparser.la
viewer_SOURCES = viewer.cpp viewer_main.cpp viewerbase.ui

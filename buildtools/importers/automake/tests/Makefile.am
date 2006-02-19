# This directory collects some classes related to
# project management for the sole purpose that they
# can be shared between parts.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
        -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets/propeditor \
        -I$(top_srcdir)/buildtools/importers/automake \
        -I$(top_builddir)/buildtools/importers/automake/tests \
        $(all_includes)

METASOURCES = AUTO

check_PROGRAMS = runner viewer testinterface
TESTS = testinterface

runner_LDFLAGS = $(all_libraries) $(LIB_KDECORE) $(KDE_RPATH)
runner_LDADD = $(top_builddir)/buildtools/importers/automake/libkdevautomakeparser.la
runner_SOURCES = runner.cpp

viewer_LDFLAGS = $(all_libraries) $(LIB_KDECORE) $(KDE_RPATH)
viewer_LDADD = $(top_builddir)/buildtools/importers/automake/libkdevautomakeparser.la
viewer_SOURCES = viewer.cpp viewer_main.cpp viewerbase.ui

testinterface_SOURCES = testinterface.cpp
testinterface_LDADD = -lQtTest_debug $(top_builddir)/buildtools/importers/automake/libkdevautomakeparser.la 
testinterface_LDFLAGS = $(all_libraries) $(LIB_KDECORE) $(KDE_RPATH)



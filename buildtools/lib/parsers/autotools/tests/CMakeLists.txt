INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets/propeditor ${CMAKE_SOURCE_DIR}/buildtools/lib/parsers/autotools )



#original Makefile.am contents follow:

## This directory collects some classes related to
## project management for the sole purpose that they
## can be shared between parts.
#
#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#        -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets/propeditor \
#        -I$(top_srcdir)/buildtools/lib/parsers/autotools \
#        -I$(top_builddir)/buildtools/lib/parsers/autotools/tests \
#        $(all_includes)
#
#METASOURCES = AUTO
#
#check_PROGRAMS = runner viewer
#
#runner_LDFLAGS = $(all_libraries) $(LIB_KDECORE) $(KDE_RPATH)
#runner_LDADD = $(top_builddir)/buildtools/lib/parsers/autotools/libkdevautotoolsparser.la
#runner_SOURCES = runner.cpp
#
#viewer_LDFLAGS = $(all_libraries) $(LIB_KDECORE) $(KDE_RPATH)
#viewer_LDADD = $(top_builddir)/buildtools/lib/parsers/autotools/libkdevautotoolsparser.la
#viewer_SOURCES = viewer.cpp viewer_main.cpp viewerbase.ui

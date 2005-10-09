# Here resides the library of general KDevelop-related utilities,
# which may be linked by any plugin or part. This is installed
# as a shared library, including header files

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkdevelop.la
libkdevelop_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 1:0
libkdevelop_la_LIBADD = $(top_builddir)/lib/util/libkdevutil.la \
	$(top_builddir)/lib/widgets/libkdevwidgets.la $(top_builddir)/lib/interfaces/libkdevinterfaces.la \
	$(top_builddir)/lib/weaver/libThreadWeaver.la $(LIB_KIO)

libkdevelop_la_SOURCES = dummy.cpp


SUBDIRS = \
    weaver \
    util \
    interfaces \
    widgets \
    projectmanager

DOXYGEN_EMPTY = YES
include ../Doxyfile.am

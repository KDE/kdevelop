# Here resides the library of general KDevelop-related utilities,
# which may be linked by any plugin or part. This is installed
# as a shared library, including header files

INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkdevelop.la
libkdevelop_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 1:0
libkdevelop_la_LIBADD = $(top_builddir)/lib/interfaces/libkdevinterfaces.la $(LIB_KIO)

#	$(top_builddir)/lib/util/libkdevutil.la $(top_builddir)/lib/widgets/propeditor/libkdevpropertyeditor.la \
#	$(top_builddir)/lib/interfaces/external/libkinterfacedesigner.la $(top_builddir)/lib/interfaces/extensions/libkdevextensions.la \
#	$(LIB_KDEVCOMPAT) -lktexteditor $(LIB_KPARTS) $(LIB_KIO)  -lkscript
libkdevelop_la_SOURCES = dummy.cpp


SUBDIRS = util interfaces widgets catalog astyle projectmanager

DOXYGEN_EMPTY = YES
include ../Doxyfile.am

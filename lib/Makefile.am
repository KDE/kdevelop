# Here resides the library of general KDevelop-related utilities,
# which may be linked by any plugin or part. This is installed
# as a shared library, including header files


INCLUDES = $(all_includes)

lib_LTLIBRARIES = libkdevelop.la
libkdevelop_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 1:0 
libkdevelop_la_LIBADD = $(top_builddir)/lib/interfaces/libkdevinterfaces.la \
			$(top_builddir)/lib/sourceinfo/libkdevsourceinfo.la \
			$(top_builddir)/lib/util/libkdevutil.la \
			$(top_builddir)/lib/project/libkdevproject.la \
			$(top_builddir)/lib/compat/libkdevcompat.la \
			-lktexteditor $(LIB_KPARTS) $(LIB_KIO)
libkdevelop_la_SOURCES = dummy.cpp


SUBDIRS = interfaces sourceinfo util project widgets qextmdi antlr structure catalog cppparser $(COMPAT_SUBDIR)


# Here resides the library of general KDevelop-related utilities,
# which may be linked by any plugin or part. This is installed
# as a shared library, including header files

INCLUDES = $(all_includes)

if include_qextmdi
QEXTMDI_SUBDIR=qextmdi
endif

if include_compat
COMPAT_SUBDIR=compat
LIB_KDEVCOMPAT=$(top_builddir)/lib/compat/libkdevcompat.la
endif

lib_LTLIBRARIES = libkdevelop.la
libkdevelop_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 1:0
libkdevelop_la_LIBADD = $(top_builddir)/lib/interfaces/libkdevinterfaces.la \
	$(top_builddir)/lib/util/libkdevutil.la $(top_builddir)/lib/widgets/propeditor/libkdevpropertyeditor.la \
	$(top_builddir)/lib/interfaces/external/libkinterfacedesigner.la $(top_builddir)/lib/interfaces/extensions/libkdevextensions.la \
	$(LIB_KDEVCOMPAT) -lktexteditor $(LIB_KPARTS) $(LIB_KIO)
libkdevelop_la_SOURCES = dummy.cpp



SUBDIRS = interfaces widgets util $(QEXTMDI_SUBDIR) antlr catalog \
	cppparser $(COMPAT_SUBDIR) astyle 

DOXYGEN_EMPTY = YES
include ../Doxyfile.am

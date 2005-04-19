#SET(QEXTMDI_SUBDIR qextmdi)

SUBDIRS( interfaces widgets util ${QEXTMDI_SUBDIR} antlr catalog cppparser ${COMPAT_SUBDIR} astyle  )

SET(kdevelop_LIB_SRCS
dummy.cpp
)

ADD_LIBRARY(kdevelop SHARED ${kdevelop_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevelop
${QT_LIBRARIES} kdecore kdeui kscript ktexteditor
${CMAKE_BINARY_DIR}/lib/interfaces/libkdevinterfaces.a
${CMAKE_BINARY_DIR}/lib/interfaces/external/libkinterfacedesigner.so
${CMAKE_BINARY_DIR}/lib/interfaces/extensions/libkdevextensions.a
${CMAKE_BINARY_DIR}/lib/util/libkdevutil.a
${CMAKE_BINARY_DIR}/lib/widgets/propeditor/libkdevpropertyeditor.so
)

#original Makefile.am contents follow:

## Here resides the library of general KDevelop-related utilities,
## which may be linked by any plugin or part. This is installed
## as a shared library, including header files
#
#INCLUDES = $(all_includes)
#
#if include_qextmdi
#QEXTMDI_SUBDIR=qextmdi
#endif
#
#if include_compat
#COMPAT_SUBDIR=compat
#LIB_KDEVCOMPAT=$(top_builddir)/lib/compat/libkdevcompat.la
#endif
#
#lib_LTLIBRARIES = libkdevelop.la
#libkdevelop_la_LDFLAGS = -no-undefined $(all_libraries) -version-info 1:0
#libkdevelop_la_LIBADD = $(top_builddir)/lib/interfaces/libkdevinterfaces.la \
#	$(top_builddir)/lib/util/libkdevutil.la $(top_builddir)/lib/widgets/propeditor/libkdevpropertyeditor.la \
#	$(top_builddir)/lib/interfaces/external/libkinterfacedesigner.la $(top_builddir)/lib/interfaces/extensions/libkdevextensions.la \
#	$(LIB_KDEVCOMPAT) -lktexteditor $(LIB_KPARTS) $(LIB_KIO)  -lkscript
#libkdevelop_la_SOURCES = dummy.cpp
#
#
#
#SUBDIRS = interfaces widgets util $(QEXTMDI_SUBDIR) antlr catalog \
#	cppparser $(COMPAT_SUBDIR) astyle
#
#DOXYGEN_EMPTY = YES
#include ../Doxyfile.am

#######################
#previous CMakeLists.txt contents follow:

#
#SUBDIRS(interfaces widgets util antlr catalog cppparser astyle)
#
##SUBDIRS(interfaces widgets util antlr catalog cppparser astyle)

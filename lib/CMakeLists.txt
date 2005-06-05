#SET(QEXTMDI_SUBDIR qextmdi)

ADD_SUBDIRECTORY( interfaces )
ADD_SUBDIRECTORY( widgets )
ADD_SUBDIRECTORY( util )
ADD_SUBDIRECTORY( antlr )
ADD_SUBDIRECTORY( catalog )
ADD_SUBDIRECTORY( cppparser )
ADD_SUBDIRECTORY( astyle )
IF(COMPAT_SUBDIR)
  ADD_SUBDIRECTORY( ${COMPAT_SUBDIR} )
ENDIF(COMPAT_SUBDIR)
IF(QEXTMDI_SUBDIR)
  ADD_SUBDIRECTORY( ${QEXTMDI_SUBDIR} )
ENDIF(QEXTMDI_SUBDIR)

SET(kdevelop_LIB_SRCS
dummy.cpp
)

ADD_LIBRARY(kdevelop SHARED ${kdevelop_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevelop
${QT_AND_KDECORE_LIBS} kdeui kscript ktexteditor DCOP kio kparts
kinterfacedesigner
kdevpropertyeditor
)

KDE_TARGET_LINK_CONV_LIBRARIES(kdevelop
kdevinterfaces
kdevextensions
kdevutil
kdevdcopextensions
)

INSTALL_TARGETS(/lib kdevelop)

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

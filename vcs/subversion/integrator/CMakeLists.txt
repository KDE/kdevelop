INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/extras ${CMAKE_SOURCE_DIR}/lib/util )

SET(subversionintegrator_PART_SRCS
subversionintegrator.cpp
svnintegratordlg.cpp
)

SET( subversionintegrator_UI
svnintegratordlgbase.ui
)

KDE_AUTOMOC(${subversionintegrator_PART_SRCS})

KDE_ADD_UI_FILES(subversionintegrator_PART_SRCS ${subversionintegrator_UI} )

ADD_LIBRARY(subversionintegrator MODULE ${subversionintegrator_PART_SRCS})

TARGET_LINK_LIBRARIES(subversionintegrator ${QT_AND_KDECORE_LIBS} kdeui kio kparts kdevelop kdevextras)

KDE_CREATE_LIBTOOL_FILE(subversionintegrator)

INSTALL_TARGETS(/lib/kde3 subversionintegrator)
INSTALL_FILES(/share/services FILES kdevsubversionintegrator.desktop )

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util \
#	$(all_includes)
#METASOURCES = AUTO
#
#if include_vcscompat
#NETACCESS_LIB = $(top_builddir)/vcs/compat/netaccess/libnetaccess.la
#endif
#
#kde_module_LTLIBRARIES = libsubversionintegrator.la
#libsubversionintegrator_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
#libsubversionintegrator_la_LIBADD =\
#	$(top_builddir)/lib/interfaces/extras/libkdevextras.la\
#	$(top_builddir)/lib/libkdevelop.la $(NETACCESS_LIB)
#kde_services_DATA = kdevsubversionintegrator.desktop
#noinst_HEADERS = subversionintegrator.h svnintegratordlg.h
#libsubversionintegrator_la_SOURCES = subversionintegrator.cpp \
#	svnintegratordlgbase.ui svnintegratordlg.cpp

SET(subversionintegrator_PART_SRCS
subversionintegrator.cpp
svnintegratordlg.cpp
)

KDE_AUTOMOC(${subversionintegrator_PART_SRCS})

SET(
svnintegratordlgbase.ui
)

KDE_ADD_UI_FILES(subversionintegrator_PART_SRCS ${subversionintegrator_UI}

ADD_LIBRARY(subversionintegrator SHARED ${subversionintegrator_PART_SRCS})



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

#######################
#previous CMakeLists.txt contents follow:


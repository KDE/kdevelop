INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/extras ${CMAKE_SOURCE_DIR}/lib/util )

SET(clearcaseintegrator_PART_SRCS
clearcaseintegrator.cpp
ccintegratordlg.cpp
)

KDE_AUTOMOC(${clearcaseintegrator_PART_SRCS})

SET( clearcaseintegrator_UI
ccintegratordlgbase.ui
)

KDE_ADD_UI_FILES(clearcaseintegrator_PART_SRCS ${clearcaseintegrator_UI} )

ADD_LIBRARY(clearcaseintegrator MODULE ${clearcaseintegrator_PART_SRCS})

TARGET_LINK_LIBRARIES(clearcaseintegrator ${QT_LIBRARIES} kdecore kdevextras kdevelop)

KDE_CREATE_LIBTOOL_FILE(clearcaseintegrator)

INSTALL_TARGETS(/lib/kde3 clearcaseintegrator)
INSTALL_FILES(/share/services FILES kdevclearcaseintegrator.desktop )

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util \
#	$(all_includes)
#METASOURCES = AUTO
#kde_module_LTLIBRARIES = libclearcaseintegrator.la
#libclearcaseintegrator_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
#libclearcaseintegrator_la_LIBADD =\
#	$(top_builddir)/lib/interfaces/extras/libkdevextras.la\
#	$(top_builddir)/lib/libkdevelop.la
#kde_services_DATA = kdevclearcaseintegrator.desktop
#noinst_HEADERS = clearcaseintegrator.h ccintegratordlg.h
#libclearcaseintegrator_la_SOURCES = clearcaseintegrator.cpp \
#	ccintegratordlgbase.ui ccintegratordlg.cpp

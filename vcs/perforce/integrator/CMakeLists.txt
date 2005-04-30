INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/extras ${CMAKE_SOURCE_DIR}/lib/util )

SET(perforceintegrator_PART_SRCS
perforceintegrator.cpp
pfintegratordlg.cpp
)

SET( perforceintegrator_UI
pfintegratordlgbase.ui
)

KDE_AUTOMOC(${perforceintegrator_PART_SRCS})

KDE_ADD_UI_FILES(perforceintegrator_PART_SRCS ${perforceintegrator_UI} )

ADD_LIBRARY(perforceintegrator SHARED ${perforceintegrator_PART_SRCS})

TARGET_LINK_LIBRARIES(perforceintegrator ${QT_LIBRARIES} kdecore kdeui kio kparts ${CMAKE_BINARY_DIR}/lib/libkdevelop.so kdevextras)


#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util \
#	$(all_includes)
#METASOURCES = AUTO
#kde_module_LTLIBRARIES = libperforceintegrator.la
#libperforceintegrator_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
#libperforceintegrator_la_LIBADD =\
#	$(top_builddir)/lib/interfaces/extras/libkdevextras.la\
#	$(top_builddir)/lib/libkdevelop.la
#kde_services_DATA = kdevperforceintegrator.desktop
#noinst_HEADERS = perforceintegrator.h pfintegratordlg.h
#libperforceintegrator_la_SOURCES = perforceintegrator.cpp \
#	pfintegratordlgbase.ui pfintegratordlg.cpp

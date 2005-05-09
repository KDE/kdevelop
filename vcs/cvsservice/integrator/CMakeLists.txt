INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/interfaces/extras ${CMAKE_SOURCE_DIR}/lib/util )

SET(cvsserviceintegrator_PART_SRCS
cvsserviceintegrator.cpp
integratordlg.cpp
)

SET( cvsserviceintegrator_UI
integratordlgbase.ui
fetcherdlgbase.ui
initdlg.ui
)

KDE_AUTOMOC(${cvsserviceintegrator_PART_SRCS})

KDE_ADD_UI_FILES(cvsserviceintegrator_PART_SRCS ${cvsserviceintegrator_UI} )

ADD_LIBRARY(cvsserviceintegrator SHARED ${cvsserviceintegrator_PART_SRCS})

TARGET_LINK_LIBRARIES(cvsserviceintegrator ${QT_LIBRARIES} kdecore kdeui kio kdevextras cvsservice kdevelop)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util \
#	$(all_includes)
#METASOURCES = AUTO
#kde_module_LTLIBRARIES = libcvsserviceintegrator.la
#libcvsserviceintegrator_la_LDFLAGS = -avoid-version -no-undefined $(all_libraries)
#libcvsserviceintegrator_la_LIBADD = \
#	$(top_builddir)/lib/interfaces/extras/libkdevextras.la $(top_builddir)/lib/libkdevelop.la -lcvsservice
#noinst_HEADERS = cvsserviceintegrator.h integratordlg.h
#libcvsserviceintegrator_la_SOURCES = cvsserviceintegrator.cpp \
#	integratordlgbase.ui fetcherdlgbase.ui integratordlg.cpp initdlg.ui
#kde_services_DATA = kdevcvsserviceintegrator.desktop

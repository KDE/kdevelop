INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/languages/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces )

SET(kdevqtimporter_LIB_SRCS
kdevqtimporter.cpp
settingsdialog.cpp
)

SET( kdevqtimporter_UI
settingsdialogbase.ui
)

KDE_AUTOMOC(${kdevqtimporter_LIB_SRCS})

KDE_ADD_UI_FILES(kdevqtimporter_LIB_SRCS ${kdevqtimporter_UI} )

ADD_LIBRARY(kdevqtimporter MODULE ${kdevqtimporter_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevqtimporter ${KDEV_LANG_LIBS} lang_interfaces)

KDE_CREATE_LIBTOOL_FILE(kdevqtimporter)

INSTALL_TARGETS(/lib/kde3 kdevqtimporter)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/languages/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES =  libkdevqtimporter.la
#
#noinst_HEADERS = kdevqtimporter.h settingsdialog.h
#libkdevqtimporter_la_SOURCES = kdevqtimporter.cpp settingsdialogbase.ui settingsdialog.cpp
#kde_services_DATA =  kdevqtimporter.desktop
#libkdevqtimporter_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/languages/lib/interfaces/liblang_interfaces.la
#libkdevqtimporter_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)

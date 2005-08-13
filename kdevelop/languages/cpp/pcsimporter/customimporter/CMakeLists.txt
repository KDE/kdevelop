INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/languages/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces )

SET(kdevcustompcsimporter_LIB_SRCS
kdevcustomimporter.cpp
settingsdialog.cpp
)

SET( kdevcustompcsimporter_UI
settingsdialogbase.ui
)

KDE_AUTOMOC(${kdevcustompcsimporter_LIB_SRCS})

KDE_ADD_UI_FILES(kdevcustompcsimporter_LIB_SRCS ${kdevcustompcsimporter_UI} )

ADD_LIBRARY(kdevcustompcsimporter MODULE ${kdevcustompcsimporter_LIB_SRCS})

TARGET_LINK_LIBRARIES(kdevcustompcsimporter kdevelop lang_interfaces)

KDE_CREATE_LIBTOOL_FILE(kdevcustomimporter)

INSTALL_TARGETS(/lib/kde3 kdevcustomimporter)

#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/languages/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES =  libkdevcustompcsimporter.la
#
#
#noinst_HEADERS =  kdevcustomimporter.h
#libkdevcustompcsimporter_la_SOURCES = kdevcustomimporter.cpp settingsdialog.cpp settingsdialogbase.ui
#libkdevcustompcsimporter_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/languages/lib/interfaces/liblang_interfaces.la
#libkdevcustompcsimporter_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
#kde_services_DATA = kdevpcscustomimporter.desktop

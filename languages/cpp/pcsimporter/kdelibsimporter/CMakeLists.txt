SET(kdevkdelibsimporter_LIB_SRCS
kdevkdelibsimporter.cpp
settingsdialog.cpp
)

KDE_AUTOMOC(${kdevkdelibsimporter_LIB_SRCS})

SET(
settingsdialogbase.ui
)

KDE_ADD_UI_FILES(kdevkdelibsimporter_LIB_SRCS ${kdevkdelibsimporter_UI}

ADD_LIBRARY(kdevkdelibsimporter SHARED ${kdevkdelibsimporter_LIB_SRCS})



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/languages/lib/interfaces \
#	-I$(top_srcdir)/lib/interfaces $(all_includes)
#METASOURCES = AUTO
#lib_LTLIBRARIES =  libkdevkdelibsimporter.la
#
#noinst_HEADERS = kdevkdelibsimporter.h settingsdialog.h
#libkdevkdelibsimporter_la_SOURCES = kdevkdelibsimporter.cpp settingsdialogbase.ui settingsdialog.cpp
#kde_services_DATA =  kdevkdelibsimporter.desktop
#libkdevkdelibsimporter_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/languages/lib/interfaces/liblang_interfaces.la
#libkdevkdelibsimporter_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)

#######################
#previous CMakeLists.txt contents follow:


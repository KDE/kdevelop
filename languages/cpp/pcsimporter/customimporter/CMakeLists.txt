SET(kdevcustompcsimporter_LIB_SRCS
kdevcustomimporter.cpp
settingsdialog.cpp
)

KDE_AUTOMOC(${kdevcustompcsimporter_LIB_SRCS})

SET(
settingsdialogbase.ui
)

KDE_ADD_UI_FILES(kdevcustompcsimporter_LIB_SRCS ${kdevcustompcsimporter_UI}

ADD_LIBRARY(kdevcustompcsimporter SHARED ${kdevcustompcsimporter_LIB_SRCS})



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

#######################
#previous CMakeLists.txt contents follow:


INCLUDES = -I$(top_srcdir)/lib/interfaces $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES =  libkdevcustomimporter.la


noinst_HEADERS =  kdevcustomimporter.h
libkdevcustomimporter_la_SOURCES = kdevcustomimporter.cpp settingsdialog.cpp settingsdialogbase.ui
libkdevcustomimporter_la_LIBADD = $(top_builddir)/lib/libkdevelop.la
libkdevcustomimporter_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
kde_services_DATA = kdevpcscustomimporter.desktop

INCLUDES = -I$(top_srcdir)/languages/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces $(all_includes)
METASOURCES = AUTO
lib_LTLIBRARIES =  libkdevcustompcsimporter.la


noinst_HEADERS =  kdevcustomimporter.h
libkdevcustompcsimporter_la_SOURCES = kdevcustomimporter.cpp settingsdialog.cpp settingsdialogbase.ui
libkdevcustompcsimporter_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/languages/lib/interfaces/liblang_interfaces.la
libkdevcustompcsimporter_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
kde_services_DATA = kdevpcscustomimporter.desktop

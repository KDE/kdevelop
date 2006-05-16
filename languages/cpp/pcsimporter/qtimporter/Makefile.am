INCLUDES = -I$(top_srcdir)/languages/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES =  libkdevqtimporter.la

noinst_HEADERS = kdevqtimporter.h settingsdialog.h
libkdevqtimporter_la_SOURCES = kdevqtimporter.cpp settingsdialogbase.ui settingsdialog.cpp
kde_services_DATA =  kdevqtimporter.desktop
libkdevqtimporter_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/languages/lib/interfaces/liblang_interfaces.la
libkdevqtimporter_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)

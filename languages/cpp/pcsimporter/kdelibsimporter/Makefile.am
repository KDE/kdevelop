INCLUDES = -I$(top_srcdir)/languages/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES =  libkdevkdelibsimporter.la

noinst_HEADERS = kdevkdelibsimporter.h settingsdialog.h
libkdevkdelibsimporter_la_SOURCES = kdevkdelibsimporter.cpp settingsdialogbase.ui settingsdialog.cpp
kde_services_DATA =  kdevkdelibsimporter.desktop
libkdevkdelibsimporter_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/languages/lib/interfaces/liblang_interfaces.la
libkdevkdelibsimporter_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)

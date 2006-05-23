INCLUDES = -I$(top_srcdir)/languages/lib/interfaces \
        -I$(top_srcdir)/lib/interfaces $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES =  libkdevqt4importer.la

noinst_HEADERS = kdevqt4importer.h settingsdialog.h
libkdevqt4importer_la_SOURCES = kdevqt4importer.cpp settingsdialogbase.ui settingsdialog.cpp
kde_services_DATA =  kdevqt4importer.desktop
libkdevqt4importer_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
        $(top_builddir)/languages/lib/interfaces/liblang_interfaces.la
libkdevqt4importer_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)

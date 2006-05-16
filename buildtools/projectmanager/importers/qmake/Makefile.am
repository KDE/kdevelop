INCLUDES = -I$(top_srcdir)/buildtools/lib/parsers/qmake \
	-I$(top_srcdir)/buildtools/projectmanager/interfaces -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES = libqmakeimporter.la
libqmakeimporter_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
noinst_HEADERS = kdevqmakeeditor.h qmakeprojectmodel.h modelcreator.h \
	projectconfigurationdlg.h
libqmakeimporter_la_SOURCES = kdevqmakeeditor.cpp qmakeprojectmodel.cpp \
	modelcreator.cpp projectconfigurationdlgbase.ui projectconfigurationdlg.cpp
kde_services_DATA = kdevqmakeimporter.desktop
libqmakeimporter_la_LIBADD = \
	$(top_builddir)/buildtools/projectmanager/interfaces/libprojectmanager_interfaces.la $(top_builddir)/buildtools/lib/parsers/qmake/libkdevqmakeparser.la

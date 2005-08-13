INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevfilelist.la
libkdevfilelist_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevfilelist_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevfilelist_la_SOURCES = filelist_widget.cpp filelist_item.cpp \
			projectviewprojectconfigbase.ui projectviewprojectconfig.cpp projectviewpart.cpp toolbarguibuilder.cpp \
	projectviewconfigbase.ui projectviewconfig.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevfilelist.desktop

rcdir = $(kde_datadir)/kdevfilelist

noinst_HEADERS = filelist_item.h filelist_widget.h projectviewprojectconfig.h \
				projectviewpart.h toolbarguibuilder.h projectviewconfig.h fileinfo.h
rc_DATA = kdevfilelist.rc

INCLUDES = -I$(top_builddir)/lib/project -I$(top_srcdir)/lib/external_interfaces -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/project -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevpascalproject.la
libkdevpascalproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevpascalproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevpascalproject_la_SOURCES = pascalproject_part.cpp pascalproject_widget.cpp pascalproject_optionsdlgbase.ui pascalprojectoptionsdlg.cpp pascalglobaloptionsdlg.cpp service.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpascalproject.desktop

rcdir = $(kde_datadir)/kdevpascalproject
rc_DATA = kdevpascalproject.rc





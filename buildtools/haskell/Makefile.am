INCLUDES = -I$(top_builddir)/lib/project -I$(top_srcdir)/lib/external_interfaces -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/project -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevhaskellproject.la
libkdevhaskellproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevhaskellproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevhaskellproject_la_SOURCES = haskellproject_part.cpp haskellproject_optionsdlgbase.ui haskellprojectoptionsdlg.cpp
# haskellproject_widget.cpp haskellproject_optionsdlgbase.ui haskellprojectoptionsdlg.cpp haskellglobaloptionsdlg.cpp service.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevhaskellproject.desktop

rcdir = $(kde_datadir)/kdevhaskellproject
rc_DATA = kdevhaskellproject.rc

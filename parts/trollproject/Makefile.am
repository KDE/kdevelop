# Here resides the troll project part for tmake or qmake

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevtrollproject.la
libkdevtrollproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevtrollproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevtrollproject_la_SOURCES = caret.cpp\
                                 filebuffer.cpp\
				 projectconfigurationdlgbase.ui \
				 projectrunoptionsdlgbase.ui \
				 filepropertybase.ui \
				 filepropertydlg.cpp \
				 projectconfigurationdlg.cpp \
				 projectrunoptionsdlg.cpp \
				 trollprojectpart.cpp \
				 trollprojectwidget.cpp


METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevtrollproject.desktop

rcdir = $(kde_datadir)/kdevtrollproject
rc_DATA = kdevtrollproject.rc

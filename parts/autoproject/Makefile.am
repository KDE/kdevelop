# Here resides the automake project part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

lib_LTLIBRARIES = libkdevautoproject.la
libkdevautoproject_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevautoproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevautoproject_la_SOURCES = autoprojectfactory.cpp \
				autoprojectpart.cpp \
				autoprojectwidget.cpp \
				projectoptionsdlg.cpp \
				subprojectoptionsdlg.cpp \
				targetoptionsdlg.cpp \
				addtargetdlg.cpp \
				addfiledlg.cpp \
				misc.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevautoproject.desktop

rcdir = $(kde_datadir)/gideon
rc_DATA = kdevautoproject.rc

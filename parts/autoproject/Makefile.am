# Here resides the automake project part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevautoproject.la
libkdevautoproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevautoproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevautoproject_la_SOURCES = autoprojectfactory.cpp \
				autoprojectpart.cpp \
				autoprojectwidget.cpp \
				projectoptionswidget.cpp \
				subprojectoptionsdlg.cpp \
				subprojectoptionsdlgbase.ui \
				targetoptionsdlg.cpp \
				targetoptionsdlgbase.ui \
				addservicedlg.cpp \
				addservicedlgbase.ui \
				addtargetdlg.cpp \
				addfiledlg.cpp \
				removefiledlg.cpp \
				addprefixdlg.cpp \
				misc.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevautoproject.desktop

rcdir = $(kde_datadir)/kdevautoproject
rc_DATA = kdevautoproject.rc

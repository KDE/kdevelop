# Here resides the automake project part.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	   -I$(top_srcdir)/lib/project -I$(top_builddir)/lib/project \
	   -I$(top_srcdir)/lib/util \
	   $(all_includes)

kde_module_LTLIBRARIES = libkdevautoproject.la
libkdevautoproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevautoproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevautoproject_la_SOURCES = kfilednddetailview.cpp autoprojectpart.cpp autoprojectwidget.cpp configureoptionswidget.cpp configureoptionswidgetbase.ui subprojectoptionsdlg.cpp subprojectoptionsdlgbase.ui targetoptionsdlg.cpp targetoptionsdlgbase.ui addservicedlg.cpp addservicedlgbase.ui addapplicationdlg.cpp addapplicationdlgbase.ui addtargetdlg.cpp addtargetdlgbase.ui addsubprojectdlg.cpp addfiledlgbase.ui addfiledlg.cpp removefiledlgbase.ui removefiledlg.cpp importexistingdlgbase.ui importexistingdlg.cpp addicondlgbase.ui addicondlg.cpp addtranslationdlg.cpp addprefixdlg.cpp fileselectorwidget.cpp misc.cpp kfiledndiconview.cpp addsubprojectdlgbase.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevautoproject.desktop kdevkdeautoproject.desktop

rcdir = $(kde_datadir)/kdevautoproject
rc_DATA = kdevautoproject.rc


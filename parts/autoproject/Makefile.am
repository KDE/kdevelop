# Here resides the automake project part.

INCLUDES = -I$(top_srcdir)/lib/interfaces \
	   -I$(top_srcdir)/lib/project -I$(top_builddir)/lib/project \
	   -I$(top_srcdir)/lib/util \
	   $(all_includes)

kde_module_LTLIBRARIES = libkdevautoproject.la
libkdevautoproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevautoproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevautoproject_la_SOURCES = autoprojectpart.cpp autoprojectwidget.cpp configureoptionswidget.cpp configureoptionswidgetbase.ui subprojectoptionsdlg.cpp subprojectoptionsdlgbase.ui targetoptionsdlg.cpp targetoptionsdlgbase.ui addservicedlg.cpp addservicedlgbase.ui addapplicationdlg.cpp addapplicationdlgbase.ui addtargetdlg.cpp addtargetdlgbase.ui addsubprojectdlg.cpp addfiledlgbase.ui addfiledlg.cpp removefiledlgbase.ui removefiledlg.cpp addicondlgbase.ui addicondlg.cpp addtranslationdlg.cpp addprefixdlg.cpp kfilednddetailview.cpp kfiledndiconview.cpp fileselectorwidget.cpp misc.cpp addsubprojectdlgbase.ui removetargetdlg.h removetargetdlg.cpp removetargetdlgbase.ui choosetargetdialog.h choosetargetdialog.cpp choosetargetdlgbase.ui addexistingdlgbase.ui addexistingfilesdlg.cpp addexistingfilesdlg.h addexistingdirectoriesdlg.h addexistingdirectoriesdlg.cpp kimporticonview.h kimporticonview.cpp autosubprojectview.h autosubprojectview.cpp autodetailsview.h autodetailsview.cpp autolistviewitems.cpp autolistviewitems.h automakemanager.cpp automakemanager.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevautoproject.desktop kdevkdeautoproject.desktop

rcdir = $(kde_datadir)/kdevautoproject
rc_DATA = kdevautoproject.rc


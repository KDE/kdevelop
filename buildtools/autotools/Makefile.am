# Here resides the automake project part.

INCLUDES = -I$(top_srcdir)/buildtools/lib -I$(top_srcdir)/buildtools/lib/base \
	-I$(top_srcdir)/buildtools/lib/widgets -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_builddir)/buildtools/lib/widgets \
	$(all_includes)

kde_module_LTLIBRARIES = libkdevautoproject.la
libkdevautoproject_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevautoproject_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/interfaces/extras/libkdevextras.la $(top_builddir)/buildtools/lib/widgets/libkdevbuildtoolswidgets.la \
	$(top_builddir)/buildtools/lib/base/libkdevbuildbase.la

libkdevautoproject_la_SOURCES = autoprojectpart.cpp autoprojectwidget.cpp \
		configureoptionswidget.cpp configureoptionswidgetbase.ui subprojectoptionsdlg.cpp \
		subprojectoptionsdlgbase.ui targetoptionsdlg.cpp targetoptionsdlgbase.ui addservicedlg.cpp \
		addservicedlgbase.ui addapplicationdlg.cpp addapplicationdlgbase.ui addtargetdlg.cpp \
		addtargetdlgbase.ui addsubprojectdlg.cpp addfiledlgbase.ui addfiledlg.cpp \
		removefiledlgbase.ui removefiledlg.cpp addicondlgbase.ui addicondlg.cpp \
		addtranslationdlg.cpp addprefixdlg.cpp kfilednddetailview.cpp kfiledndiconview.cpp \
		fileselectorwidget.cpp misc.cpp addsubprojectdlgbase.ui removetargetdlg.cpp \
		removetargetdlgbase.ui choosetargetdialog.cpp choosetargetdlgbase.ui addexistingdlgbase.ui \
		addexistingfilesdlg.cpp addexistingdirectoriesdlg.cpp kimporticonview.cpp \
		autosubprojectview.cpp autodetailsview.cpp autolistviewitems.cpp automakemanager.cpp \
	managecustomcommandsbase.ui managecustomcommand.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevautoproject.desktop kdevkdeautoproject.desktop

rcdir = $(kde_datadir)/kdevautoproject
rc_DATA = kdevautoproject.rc

noinst_HEADERS = managecustomcommand.h

# Here resides the C/C++ support part.

SUBDIRS = templates subclassing_template newclass_templates file_templates \
	app_templates compiler pcsimporter debugger doc

INCLUDES = -I$(top_srcdir)/languages/lib/designer_integration \
	-I$(top_srcdir)/languages/lib/interfaces -I$(top_srcdir)/lib/catalog -I$(top_srcdir)/lib/compat \
	-I$(top_srcdir)/lib/cppparser -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external -I$(top_srcdir)/lib/util \
	-I$(top_builddir)/languages/lib/designer_integration -I$(top_builddir)/bdb/build_unix -I$(top_srcdir)/bdb/include \
	$(all_includes)

kde_module_LTLIBRARIES = libkdevcppsupport.la
libkdevcppsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -L$(top_srcdir)/bdb/dist
libkdevcppsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/catalog/libkdevcatalog.la $(top_builddir)/lib/cppparser/libkdevcppparser.la \
	$(top_builddir)/languages/lib/interfaces/liblang_interfaces.la \
	$(top_builddir)/languages/lib/designer_integration/libdesignerintegration.la $(top_builddir)/bdb/build_unix/libdb.a

libkdevcppsupport_la_SOURCES = cppsupportpart.cpp ccconfigwidget.cpp \
		ccconfigwidgetbase.ui kdevdriver.cpp cppcodecompletion.cpp problemreporter.cpp \
		configproblemreporter.ui backgroundparser.cpp ast_utils.cpp store_walker.cpp \
		KDevCppSupportIface.cpp KDevCppSupportIface.skel cppsupportfactory.cpp tag_creator.cpp \
		codeinformationrepository.cpp doxydoc.cpp cppcodecompletionconfig.cpp cppnewclassdlgbase.ui \
		cppnewclassdlg.cpp classgeneratorconfig.cpp classgeneratorconfigbase.ui subclassingdlg.cpp \
		subclassingdlgbase.ui addattributedialogbase.ui addmethoddialogbase.ui addattributedialog.cpp \
		addmethoddialog.cpp cppsupport_utils.cpp createpcsdialogbase.ui createpcsdialog.cpp \
		cppimplementationwidget.cpp qtdesignercppintegration.cpp creategettersetterconfiguration.cpp \
	creategettersetterconfiguration.h creategettersetterdialog.cpp creategettersetterdialog.h \
	creategettersetter.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop kdevcsupport.desktop

rcdir = $(kde_datadir)/kdevcppsupport
rc_DATA = kdevcppsupport.rc

templatedir = ${kde_datadir}/kdevabbrev/templates
template_DATA = cpptemplates

configurationdir = ${kde_datadir}/kdevcppsupport
configuration_DATA = configuration

AM_CXXFLAGS = -DAST_DEBUG

bin_PROGRAMS = r++
r___LDFLAGS = $(all_libraries)
r___SOURCES = main.cpp
r___DEPENDENCIES = libkdevcppsupport.la
r___LDADD = libkdevcppsupport.la $(top_builddir)/lib/catalog/libkdevcatalog.la $(top_builddir)/lib/cppparser/libkdevcppparser.la
noinst_HEADERS = cppimplementationwidget.h qtdesignercppintegration.h

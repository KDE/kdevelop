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
libkdevcppsupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevcppsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/catalog/libkdevcatalog.la $(top_builddir)/lib/cppparser/libkdevcppparser.la \
	$(top_builddir)/languages/lib/interfaces/liblang_interfaces.la \
	$(top_builddir)/languages/lib/designer_integration/libdesignerintegration.la $(top_builddir)/lib/interfaces/libkdevinterfaces.la

libkdevcppsupport_la_SOURCES = KDevCppSupportIface.cpp KDevCppSupportIface.skel \
	addattributedialog.cpp addattributedialogbase.ui addmethoddialog.cpp addmethoddialogbase.ui \
	ast_utils.cpp backgroundparser.cpp ccconfigwidget.cpp \
	ccconfigwidgetbase.ui classgeneratorconfig.cpp classgeneratorconfigbase.ui \
	codeinformationrepository.cpp completiondebug.cpp configproblemreporter.ui cppcodecompletion.cpp \
	cppcodecompletionconfig.cpp cppevaluation.cpp cppimplementationwidget.cpp cppnewclassdlg.cpp \
	cppnewclassdlgbase.ui cppsplitheadersourceconfig.cpp cppsupport_utils.cpp \
	cppsupportfactory.cpp cppsupportpart.cpp creategettersetter.ui \
	creategettersetterconfiguration.cpp creategettersetterconfiguration.h creategettersetterdialog.cpp \
	creategettersetterdialog.h createpcsdialog.cpp createpcsdialogbase.ui doxydoc.cpp includefiles.cpp \
	kdevdriver.cpp problemreporter.cpp qtbuildconfig.cpp qtdesignercppintegration.cpp \
	setuphelper.cpp simplecontext.cpp simpletype.cpp simpletypecatalog.cpp \
	simpletypefunction.cpp simpletypenamespace.cpp store_walker.cpp storeconverter.cpp \
	stringhelpers.cpp subclassingdlg.cpp subclassingdlgbase.ui tag_creator.cpp typedesc.cpp

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

#bin_PROGRAMS = r++
#r___LDFLAGS = $(all_libraries)
#r___SOURCES = main.cpp
#r___DEPENDENCIES = libkdevcppsupport.la
#r___LDADD = libkdevcppsupport.la $(top_builddir)/lib/catalog/libkdevcatalog.la $(top_builddir)/lib/cppparser/libkdevcppparser.la
noinst_HEADERS = cppevaluation.h cppimplementationwidget.h \
	cppsplitheadersourceconfig.h declarationinfo.h expressioninfo.h includefiles.h \
	qtdesignercppintegration.h setuphelper.h simplecontext.h simpletypecatalog.h simpletypefunction.h \
	simpletypenamespace.h storeconverter.h

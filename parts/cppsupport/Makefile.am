# Here resides the C/C++ support part.

SUBDIRS = templates subclassing_template newclass_templates

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/compat  -I$(top_srcdir)/lib/sourceinfo -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/catalog -I$(top_srcdir)/lib/cppparser $(all_includes)

kde_module_LTLIBRARIES = libkdevcppsupport.la
libkdevcppsupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN)
libkdevcppsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/catalog/libkdevcatalog.la $(top_builddir)/lib/cppparser/libkdevcppparser.la

libkdevcppsupport_la_SOURCES = cppsupportpart.cpp ccconfigwidget.cpp ccconfigwidgetbase.ui cppnewclassdlgbase.ui cppnewclassdlg.cpp kdevdriver.cpp cppcodecompletion.cpp problemreporter.cpp configproblemreporter.ui backgroundparser.cpp ast_utils.cpp store_walker.cpp subclassingdlg.cpp subclassingdlgbase.ui addmethoddialogbase.ui addmethoddialog.cpp addattributedialogbase.ui addattributedialog.cpp KDevCppSupportIface.cpp KDevCppSupportIface.skel classgeneratorconfigbase.ui classgeneratorconfig.cpp cppsupportfactory.cpp tag_creator.cpp codeinformationrepository.cpp doxydoc.cpp cppcodecompletionconfig.cpp




METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop kdevcsupport.desktop

rcdir = $(kde_datadir)/kdevcppsupport
rc_DATA = kdevcppsupport.rc

templatedir = ${kde_datadir}/kdevabbrev/templates
template_DATA = cpptemplates

AM_CXXFLAGS = -DAST_DEBUG

bin_PROGRAMS = r++
r___LDFLAGS = $(all_libraries)
r___SOURCES = main.cpp
r___DEPENDENCIES = libkdevcppsupport.la
r___LDADD = libkdevcppsupport.la $(top_builddir)/lib/catalog/libkdevcatalog.la $(top_builddir)/lib/cppparser/libkdevcppparser.la

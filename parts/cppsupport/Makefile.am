# Here resides the C/C++ support part.

SUBDIRS = templates subclassing_template newclass_templates

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/sourceinfo -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/catalog $(all_includes)

kde_module_LTLIBRARIES = libkdevcppsupport.la
libkdevcppsupport_la_LDFLAGS = $(LEXLIB) $(all_libraries) $(KDE_PLUGIN)
libkdevcppsupport_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/catalog/libkdevcatalog.la

libkdevcppsupport_la_SOURCES = cppsupportpart.cpp ccconfigwidget.cpp ccconfigwidgetbase.ui cppnewclassdlgbase.ui cppnewclassdlg.cpp cppcodecompletion.cpp simpleparser.cpp kdevregexp.cpp problemreporter.cpp driver.cpp errors.cpp lexer.cpp parser.cpp ast.cpp configproblemreporter.ui backgroundparser.cpp implementmethodsdialog.ui implementmethodsdialog.ui.h cppsupport_events.h ast_utils.h ast_utils.cpp tree_parser.h tree_parser.cpp store_walker.h store_walker.cpp parser.h realtime_classbrowser.h realtime_classbrowser.cpp subclassingdlg.cpp subclassingdlgbase.ui lookup.h lookup.cpp keywords.lut.h addmethoddialogbase.ui addmethoddialog.cpp addattributedialogbase.ui addattributedialog.cpp addattributedialog.h KDevCppSupportIface.h KDevCppSupportIface.cpp KDevCppSupportIface.skel classgeneratorconfigbase.ui classgeneratorconfig.cpp classgeneratorconfig.h cppsupportfactory.cpp cppsupportfactory.h tag_creator.cpp codeinformationrepository.h codeinformationrepository.cpp

EXTRA_DIST =
DISTCLEANFILES =

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevcppsupport.desktop kdevcsupport.desktop

rcdir = $(kde_datadir)/kdevcppsupport
rc_DATA = kdevcppsupport.rc

templatedir = ${kde_datadir}/kdevabbrev/templates
template_DATA = cpptemplates
AM_CFLAGS =
AM_CXXFLAGS = -DAST_DEBUG
AM_FFLAGS =
bin_PROGRAMS =  r++
r___LDFLAGS = $(all_libraries)
r___SOURCES = main.cpp cppsupportfactory.h
r___DEPENDENCIES = libkdevcppsupport.la
r___LDADD = libkdevcppsupport.la

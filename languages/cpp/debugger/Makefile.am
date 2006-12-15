# Here resides the debugger part.

KDE_CXXFLAGS = $(USE_EXCEPTIONS)

SUBDIRS = mi
INCLUDES = -I$(top_srcdir)/languages/lib/debugger \
	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/widgets $(all_includes)

kde_module_LTLIBRARIES = libkdevdebugger.la
libkdevdebugger_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevdebugger_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libkdevwidgets.la $(LIB_KHTML) \
	$(top_builddir)/languages/lib/debugger/liblang_debugger.la \
	$(top_builddir)/languages/cpp/debugger/mi/libgdbmi_parser.la

libkdevdebugger_la_SOURCES = debuggerdcopinterface.skel debuggerpart.cpp \
    dbgcontroller.cpp gdbcontroller.cpp gdbcommand.cpp \
    gdbparser.cpp stty.cpp breakpoint.cpp variablewidget.cpp \
    gdbbreakpointwidget.cpp framestackwidget.cpp disassemblewidget.cpp \
    memviewdlg.cpp dbgpsdlg.cpp dbgtoolbar.cpp debuggerconfigwidget.cpp \
    debuggerconfigwidgetbase.ui debuggertracingdialogbase.ui \
    gdboutputwidget.cpp gdbtable.cpp debuggertracingdialog.cpp \
	label_with_double_click.cpp

METASOURCES = AUTO
KDE_ICON = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevdebugger.desktop

rcdir = $(kde_datadir)/kdevdebugger
rc_DATA = kdevdebugger.rc
noinst_HEADERS =  gdbtable.h

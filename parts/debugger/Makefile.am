# Here resides the debugger part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

kde_module_LTLIBRARIES = libkdevdebugger.la
libkdevdebugger_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevdebugger_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevdebugger_la_SOURCES = debuggerpart.cpp 			     dbgcontroller.cpp 			     gdbcontroller.cpp 			     dbgcommand.cpp 			     gdbcommand.cpp 			     gdbparser.cpp 			     stty.cpp 			     breakpoint.cpp 			     variablewidget.cpp 			     breakpointwidget.cpp 			     framestackwidget.cpp 			     disassemblewidget.cpp 			     memviewdlg.cpp 			     breakpointdlg.cpp 			     dbgpsdlg.cpp 			     dbgtoolbar.cpp 			     debuggerconfigwidget.cpp 			     debuggerconfigwidgetbase.ui gdboutputwidget.cpp gdboutputwidget.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevdebugger.desktop

rcdir = $(kde_datadir)/kdevdebugger
rc_DATA = kdevdebugger.rc

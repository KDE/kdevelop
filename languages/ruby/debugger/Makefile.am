# Here resides the debugger part.

INCLUDES = -I$(top_srcdir)/languages/lib/debugger \
	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util \
	-I$(top_srcdir)/lib/widgets $(all_includes)

kde_module_LTLIBRARIES = libkdevrbdebugger.la
libkdevrbdebugger_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevrbdebugger_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libwidgets.la $(LIB_KHTML) \
	$(top_builddir)/languages/lib/debugger/liblang_debugger.la

libkdevrbdebugger_la_SOURCES = debuggerpart.cpp dbgcontroller.cpp rdbcontroller.cpp dbgcommand.cpp rdbcommand.cpp rdbparser.cpp stty.cpp breakpoint.cpp variablewidget.cpp rdbbreakpointwidget.cpp framestackwidget.cpp dbgpsdlg.cpp dbgtoolbar.cpp rdboutputwidget.cpp rdbtable.cpp

METASOURCES = AUTO
KDE_ICON = AUTO

rubysrc_DATA = debuggee.rb
rubysrcdir = $(kde_datadir)/kdevrbdebugger

servicedir = $(kde_servicesdir)
service_DATA = kdevrbdebugger.desktop

rcdir = $(kde_datadir)/kdevrbdebugger
rc_DATA = kdevrbdebugger.rc
noinst_HEADERS =  rdbtable.h

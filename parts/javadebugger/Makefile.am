# Here resides the debugger part.

INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util $(all_includes)

lib_LTLIBRARIES = libkdevjavadebugger.la
libkdevjavadebugger_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevjavadebugger_la_LIBADD = $(top_builddir)/lib/libkdevelop.la

libkdevjavadebugger_la_SOURCES = javadebuggerfactory.cpp \
			     javadebuggerpart.cpp \
			     dbgcontroller.cpp \
			     jdbcontroller.cpp \
			     dbgcommand.cpp \
			     jdbcommand.cpp \
			     jdbparser.cpp \
			     stty.cpp \
			     breakpoint.cpp \
			     variablewidget.cpp \
			     breakpointwidget.cpp \
			     framestackwidget.cpp \
			     disassemblewidget.cpp \
			     memviewdlg.cpp \
			     breakpointdlg.cpp \
			     dbgpsdlg.cpp

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevjavadebugger.desktop

rcdir = $(kde_datadir)/kdevjavadebugger
rc_DATA = kdevjavadebugger.rc

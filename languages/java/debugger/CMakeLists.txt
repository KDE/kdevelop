SET(kdevjavadebugger_PART_SRCS
javadebuggerpart.cpp
dbgcontroller.cpp
jdbcontroller.cpp
dbgcommand.cpp
jdbcommand.cpp
jdbparser.cpp
stty.cpp
breakpoint.cpp
variablewidget.cpp
breakpointwidget.cpp
framestackwidget.cpp
disassemblewidget.cpp
memviewdlg.cpp
breakpointdlg.cpp
dbgpsdlg.cpp
)

KDE_AUTOMOC(${kdevjavadebugger_PART_SRCS})

ADD_LIBRARY(kdevjavadebugger SHARED ${kdevjavadebugger_PART_SRCS})



#original Makefile.am contents follow:

## Here resides the debugger part.
#
#INCLUDES = -I$(top_srcdir)/languages/lib/debugger \
#	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = libkdevjavadebugger.la
#libkdevjavadebugger_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevjavadebugger_la_LIBADD = $(top_builddir)/lib/libkdevelop.la \
#	$(top_builddir)/languages/lib/debugger/liblang_debugger.la
#
#libkdevjavadebugger_la_SOURCES = javadebuggerpart.cpp dbgcontroller.cpp jdbcontroller.cpp dbgcommand.cpp jdbcommand.cpp jdbparser.cpp stty.cpp breakpoint.cpp variablewidget.cpp breakpointwidget.cpp framestackwidget.cpp disassemblewidget.cpp memviewdlg.cpp breakpointdlg.cpp dbgpsdlg.cpp
#
#METASOURCES = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevjavadebugger.desktop
#
#rcdir = $(kde_datadir)/kdevjavadebugger
#rc_DATA = kdevjavadebugger.rc

#######################
#previous CMakeLists.txt contents follow:


INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/languages/lib/debugger ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets )

SET(kdevrbdebugger_PART_SRCS
debuggerpart.cpp
dbgcontroller.cpp
rdbcontroller.cpp
dbgcommand.cpp
rdbcommand.cpp
rdbparser.cpp
stty.cpp
breakpoint.cpp
variablewidget.cpp
rdbbreakpointwidget.cpp
framestackwidget.cpp
dbgpsdlg.cpp
dbgtoolbar.cpp
rdboutputwidget.cpp
rdbtable.cpp
)

KDE_AUTOMOC(${kdevrbdebugger_PART_SRCS})

ADD_LIBRARY(kdevrbdebugger SHARED ${kdevrbdebugger_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevrbdebugger ${KDEV_LANG_LIBS} lang_debugger DCOP kdevwidgets)

KDE_CREATE_LIBTOOL_FILE(kdevrbdebugger)

INSTALL_TARGETS(/lib/kde3 kdevrbdebugger)

#original Makefile.am contents follow:

## Here resides the debugger part.
#
#INCLUDES = -I$(top_srcdir)/languages/lib/debugger \
#	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevrbdebugger.la
#libkdevrbdebugger_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevrbdebugger_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libkdevwidgets.la $(LIB_KHTML) \
#	$(top_builddir)/languages/lib/debugger/liblang_debugger.la
#
#libkdevrbdebugger_la_SOURCES = debuggerpart.cpp dbgcontroller.cpp rdbcontroller.cpp dbgcommand.cpp rdbcommand.cpp rdbparser.cpp stty.cpp breakpoint.cpp variablewidget.cpp rdbbreakpointwidget.cpp framestackwidget.cpp dbgpsdlg.cpp dbgtoolbar.cpp rdboutputwidget.cpp rdbtable.cpp
#
#METASOURCES = AUTO
#KDE_ICON = AUTO
#
#rubysrc_DATA = debuggee.rb
#rubysrcdir = $(kde_datadir)/kdevrbdebugger
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevrbdebugger.desktop
#
#rcdir = $(kde_datadir)/kdevrbdebugger
#rc_DATA = kdevrbdebugger.rc
#noinst_HEADERS =  rdbtable.h

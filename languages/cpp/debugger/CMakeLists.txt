INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/languages/lib/debugger ${CMAKE_SOURCE_DIR}/lib/interfaces ${CMAKE_SOURCE_DIR}/lib/interfaces/extensions ${CMAKE_SOURCE_DIR}/lib/util ${CMAKE_SOURCE_DIR}/lib/widgets )

SET(kdevdebugger_PART_SRCS
debuggerpart.cpp
dbgcontroller.cpp
gdbcontroller.cpp
dbgcommand.cpp
gdbcommand.cpp
gdbparser.cpp
stty.cpp
breakpoint.cpp
variablewidget.cpp
gdbbreakpointwidget.cpp
framestackwidget.cpp
disassemblewidget.cpp
memviewdlg.cpp
dbgpsdlg.cpp
dbgtoolbar.cpp
debuggerconfigwidget.cpp
gdboutputwidget.cpp
gdbtable.cpp
)

SET( kdevdebugger_UI
debuggerconfigwidgetbase.ui
)

SET( kdevdebugger_DCOP_SRCS
debuggerdcopinterface.h
)

KDE_AUTOMOC(${kdevdebugger_PART_SRCS})

KDE_ADD_UI_FILES(kdevdebugger_PART_SRCS ${kdevdebugger_UI} )

KDE_ADD_DCOP_SKELS(kdevdebugger_PART_SRCS ${kdevdebugger_DCOP_SRCS})

ADD_LIBRARY(kdevdebugger MODULE ${kdevdebugger_PART_SRCS})

TARGET_LINK_LIBRARIES(kdevdebugger ${KDEV_LANG_LIBS} DCOP kdevwidgets lang_debugger)

KDE_CREATE_LIBTOOL_FILE(kdevdebugger)

INSTALL_TARGETS(/lib/kde3 kdevdebugger)

#original Makefile.am contents follow:

## Here resides the debugger part.
#
#INCLUDES = -I$(top_srcdir)/languages/lib/debugger \
#	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/util \
#	-I$(top_srcdir)/lib/widgets $(all_includes)
#
#kde_module_LTLIBRARIES = libkdevdebugger.la
#libkdevdebugger_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
#libkdevdebugger_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libkdevwidgets.la $(LIB_KHTML) \
#	$(top_builddir)/languages/lib/debugger/liblang_debugger.la
#
#libkdevdebugger_la_SOURCES = debuggerdcopinterface.skel debuggerpart.cpp dbgcontroller.cpp gdbcontroller.cpp dbgcommand.cpp gdbcommand.cpp gdbparser.cpp stty.cpp breakpoint.cpp variablewidget.cpp gdbbreakpointwidget.cpp framestackwidget.cpp disassemblewidget.cpp memviewdlg.cpp dbgpsdlg.cpp dbgtoolbar.cpp debuggerconfigwidget.cpp debuggerconfigwidgetbase.ui gdboutputwidget.cpp gdbtable.cpp
#
#METASOURCES = AUTO
#KDE_ICON = AUTO
#
#servicedir = $(kde_servicesdir)
#service_DATA = kdevdebugger.desktop
#
#rcdir = $(kde_datadir)/kdevdebugger
#rc_DATA = kdevdebugger.rc
#noinst_HEADERS =  gdbtable.h

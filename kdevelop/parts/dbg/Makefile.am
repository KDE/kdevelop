INCLUDES = -I$(top_srcdir)/kdevelop/lib/general $(all_includes)

lib_LTLIBRARIES = libkdevelopdbg.la
libkdevelopdbg_la_LDFLAGS = $(all_libraries) -version-info 1:0 $(KDE_PLUGIN)
libkdevelopdbg_la_LIBADD = $(top_builddir)/kdevelop/lib/libkdevelop.la

libkdevelopdbg_la_SOURCES = dbgmanager.cpp dbgfactory.cpp memview.cpp \
                            breakpointdialog.cpp  stty.cpp gdbparser.cpp \
                            breakpoint.cpp brkptmanager.cpp dbgcommand.cpp \
                            dbgcontroller.cpp framestack.cpp gdbcommand.cpp \
                            gdbcontroller.cpp vartree.cpp disassemble.cpp \
                            dbgpsdlg.cpp dbgtoolbar.cpp

noinst_HEADERS = dbgmanager.h dbgfactory.h memview.h breakpointdialog.h \
                 stty.h gdbparser.h breakpoint.h brkptmanager.h \
                 dbgcommand.h dbgcontroller.h framestack.h gdbcommand.h \
                 gdbcontroller.h vartree.h disassemble.h \
                 dbgpsdlg.cpp dbgtoolbar.cpp

toolbar_DATA = dbgbreak.xpm dbgnext.xpm dbgrun.xpm dbgstop.xpm  \
               dbgrestart.xpm dbgstep.xpm dbgmemview.xpm dbgwatchvar.xpm \
               dbgrunto.xpm dbgstepout.xpm dbgnextinst.xpm dbgstepinst.xpm 

toolbardir = $(kde_datadir)/kdevelop/toolbar

EXTRA_DIST = $(toolbar_DATA)

METASOURCES = AUTO

#servicedir = $(kde_servicesdir)
#service_DATA = kdevelopdbg.desktop

rcdir = $(kde_datadir)/kdevelopdbg
rc_DATA = kdevelopdbgui.rc


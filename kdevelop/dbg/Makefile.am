
INCLUDES= $(all_includes)

noinst_LIBRARIES = libdbg.a

libdbg_a_METASOURCES = USE_AUTOMOC

libdbg_a_SOURCES = memview.cpp breakpointdialog.cpp stty.cpp gdbparser.cpp breakpoint.cpp \
                   brkptmanager.cpp dbgcommand.cpp dbgcontroller.cpp \
                   framestack.cpp gdbcommand.cpp gdbcontroller.cpp vartree.cpp

noinst_HEADERS = memview.h breakpointdialog.h stty.h gdbparser.h breakpoint.h brkptmanager.h \
                 dbgcommand.h dbgcontroller.h framestack.h gdbcommand.h \
                 gdbcontroller.h vartree.h

toolbar_DATA = dbgbreak.xpm dbgnext.xpm dbgrun.xpm dbgstop.xpm  \
               dbgrestart.xpm dbgstep.xpm dbgmemview.xpm dbgwatchvar.xpm \
               dbgrunto.xpm dbgstepout.xpm

toolbardir = $(kde_datadir)/kdevelop/toolbar

EXTRA_DIST = $(toolbar_DATA)

INCLUDES = 	$(all_includes)

lib_LTLIBRARIES = libkdevelopdbg.la
libkdevelopdbg_la_LDFLAGS = $(all_libraries) -version-info 2:0

libkdevelopdbg_la_SOURCES = memview.cpp \
                            breakpointdialog.cpp  stty.cpp gdbparser.cpp \
                            breakpoint.cpp brkptmanager.cpp dbgcommand.cpp \
                            dbgcontroller.cpp framestack.cpp gdbcommand.cpp \
                            gdbcontroller.cpp vartree.cpp disassemble.cpp

noinst_HEADERS = memview.h breakpointdialog.h \
                 stty.h gdbparser.h breakpoint.h brkptmanager.h \
                 dbgcommand.h dbgcontroller.h framestack.h gdbcommand.h \
                 gdbcontroller.h vartree.h disassemble.h

EXTRA_DIST = breakpoint.cpp breakpoint.h breakpointdialog.cpp breakpointdialog.h brkptmanager.cpp brkptmanager.h dbgcommand.cpp dbgcommand.h dbgcontroller.cpp dbgcontroller.h disassemble.h framestack.cpp framestack.h gdbcommand.cpp gdbcommand.h disassemble.cpp gdbcontroller.h gdbparser.cpp gdbparser.h memview.cpp memview.h stty.cpp stty.h vartree.cpp vartree.h gdbcontroller.cpp

METASOURCES = AUTO

KDE_CXXFLAGS=-DQT_CLEAN_NAMESPACE -DQT_NO_COMPAT -DQT_NO_ASCII_CAST

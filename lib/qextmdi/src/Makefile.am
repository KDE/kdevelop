# Here resides the qextmdi library.

INCLUDES = -I$(top_srcdir)/lib/qextmdi/include -I$(top_srcdir)/lib/qextmdi/res -I$(top_srcdir)/lib/structure $(all_includes)

lib_LTLIBRARIES = libkdevelopqextmdi.la
libkdevelopqextmdi_la_LDFLAGS = $(all_libraries) -version-info 3:0
libkdevelopqextmdi_la_LIBADD = $(LIB_KPARTS)

libkdevelopqextmdi_la_SOURCES = kmdichildarea.cpp kmdichildfrm.cpp kmdichildfrmcaption.cpp \
				kmdichildview.cpp kmdimainfrm.cpp kmditaskbar.cpp kmdidockcontainer.cpp \
				kdockwidget_compat.cpp kmultitabbar.cpp \
				kmditoolviewaccessor.cpp

KDE_OPTIONS = nofinal
METASOURCES = AUTO

META_INCLUDES = $(srcdir)/../include

# Here resides the qextmdi library.

INCLUDES = -I$(top_srcdir)/lib/qextmdi/res -I$(top_srcdir)/lib/qextmdi $(all_includes)

noinst_LTLIBRARIES = libkdevelopqextmdi.la
libkdevelopqextmdi_la_LDFLAGS = $(all_libraries) -version-info 3:0
libkdevelopqextmdi_la_LIBADD = $(LIB_KPARTS) $(LIB_KUTILS)

libkdevelopqextmdi_la_SOURCES = kmdichildarea.cpp kmdichildfrm.cpp kmdichildfrmcaption.cpp \
                     kmdichildview.cpp kmdimainfrm.cpp kmditaskbar.cpp kmdidockcontainer.cpp \
                     kmditoolviewaccessor.cpp kmdiguiclient.cpp kmdidocumentviewtabwidget.cpp \
                     kmdifocuslist.cpp kdockwidget.cpp kdockwidget_private.cpp kmultitabbar.cpp

noinst_HEADERS = kmdichildarea.h kmdichildfrm.h kmdichildfrmcaption.h \
	kmdidefines.h kmdiiterator.h kmdilistiterator.h kmdimainfrm.h kmdinulliterator.h \
	kmditaskbar.h kmdichildview.h kmditoolviewaccessor.h kdockwidget.h kdockwidget_p.h \
	kdockwidget_private.h kmultitabbar.h kmultitabbar_p.h kdelibs_export.h


METASOURCES = AUTO

#AM_CXXFLAGS = -DNO_INCLUDE_MOCFILES

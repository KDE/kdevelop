# Here resides the qextmdi library.

INCLUDES = $(all_includes) -I$(top_srcdir)/kdevelop/widgets/qextmdi/include -I$(top_srcdir)/kdevelop/widgets/qextmdi/res

lib_LTLIBRARIES = libkdevelopqextmdi.la
libkdevelopqextmdi_la_LDFLAGS = $(all_libraries) -version-info 2:0
libkdevelopqextmdi_la_LIBADD  = $(LIB_KDEUI) $(LIB_KPARTS)

libkdevelopqextmdi_la_SOURCES = qextmdichildarea.cpp qextmdichildfrm.cpp qextmdichildfrmcaption.cpp qextmdichildview.cpp qextmdimainfrm.cpp qextmditaskbar.cpp

noinst_HEADERS = ../include/qextmdichildarea.h ../include/qextmdichildfrm.h ../include/qextmdichildfrmcaption.h ../include/qextmdichildview.h ../include/qextmdimainfrm.h ../include/qextmditaskbar.h ../include/qextmdiiterator.h ../include/qextmdilistiterator.h ../include/qextmdinulliterator.h

METASOURCES = AUTO

META_INCLUDES=$(srcdir)/../include

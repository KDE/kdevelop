# Here resides the qextmdi library.

INCLUDES = $(all_includes) -I../include -I../res

lib_LTLIBRARIES = libkdevqextmdi.la
libkdevqextmdi_la_LDFLAGS = $(all_libraries) -version-info 1:0

libkdevqextmdi_la_SOURCES = qextmdichildarea.cpp qextmdichildfrm.cpp qextmdichildfrmcaption.cpp qextmdichildview.cpp qextmdimainfrm.cpp qextmditaskbar.cpp

noinst_HEADERS = ../include/qextmdichildarea.h ../include/qextmdichildfrm.h ../include/qextmdichildfrmcaption.h ../include/qextmdichildview.h ../include/kextmdimainfrm.h ../include/qextmditaskbar.h ../include/qextmdiiterator.h ../include/qextmdilistiterator.h ../include/qextmdinulliterator.h

# add a dependency for every moc file to keep the src- and include- directory-structure
$(srcdir)/qextmdichildarea.cpp: moc_qextmdichildarea.cpp
moc_qextmdichildarea.cpp: ../include/qextmdichildarea.h
	$(MOC) ../include/qextmdichildarea.h -o moc_qextmdichildarea.cpp

$(srcdir)/qextmdichildfrm.cpp: moc_qextmdichildfrm.cpp
moc_qextmdichildfrm.cpp: ../include/qextmdichildfrm.h
	$(MOC) ../include/qextmdichildfrm.h -o moc_qextmdichildfrm.cpp

$(srcdir)/qextmdichildfrmcaption.cpp: moc_qextmdichildfrmcaption.cpp
moc_qextmdichildfrmcaption.cpp: ../include/qextmdichildfrmcaption.h
	$(MOC) ../include/qextmdichildfrmcaption.h -o moc_qextmdichildfrmcaption.cpp

$(srcdir)/qextmdichildview.cpp: moc_qextmdichildview.cpp
moc_qextmdichildview.cpp: ../include/qextmdichildview.h
	$(MOC) ../include/qextmdichildview.h -o moc_qextmdichildview.cpp

$(srcdir)/qextmdimainfrm.cpp: moc_kextmdimainfrm.cpp
moc_kextmdimainfrm.cpp: ../include/kextmdimainfrm.h
	$(MOC) ../include/kextmdimainfrm.h -o moc_kextmdimainfrm.cpp

$(srcdir)/qextmditaskbar.cpp: moc_qextmditaskbar.cpp
moc_qextmditaskbar.cpp: ../include/qextmditaskbar.h
	$(MOC) ../include/qextmditaskbar.h -o moc_qextmditaskbar.cpp

# Here resides the qextmdi library.

INCLUDES = $(all_includes) -I$(top_srcdir)/kdevelop/widgets/qextmdi/include -I$(top_srcdir)/kdevelop/widgets/qextmdi/res

KDE_CXXFLAGS = -DINCLUDE_MOCFILES

lib_LTLIBRARIES = libkdevqextmdi.la
libkdevqextmdi_la_LDFLAGS = $(all_libraries) -version-info 1:0

libkdevqextmdi_la_SOURCES = qextmdichildarea.cpp qextmdichildfrm.cpp qextmdichildfrmcaption.cpp qextmdichildview.cpp qextmdimainfrm.cpp qextmditaskbar.cpp

noinst_HEADERS = ../include/qextmdichildarea.h ../include/qextmdichildfrm.h ../include/qextmdichildfrmcaption.h ../include/qextmdichildview.h ../include/kextmdimainfrm.h ../include/qextmditaskbar.h ../include/qextmdiiterator.h ../include/qextmdilistiterator.h ../include/qextmdinulliterator.h

METASOURCES = AUTO

# add a dependency for every moc file to keep the src- and include- directory-structure
$(srcdir)/qextmdichildarea.cpp: qextmdichildarea.moc
qextmdichildarea.moc: ../include/qextmdichildarea.h
	$(MOC) ../include/qextmdichildarea.h -o qextmdichildarea.moc

$(srcdir)/qextmdichildfrm.cpp: qextmdichildfrm.moc
qextmdichildfrm.moc: ../include/qextmdichildfrm.h
	$(MOC) ../include/qextmdichildfrm.h -o qextmdichildfrm.moc

$(srcdir)/qextmdichildfrmcaption.cpp: qextmdichildfrmcaption.moc
qextmdichildfrmcaption.moc: ../include/qextmdichildfrmcaption.h
	$(MOC) ../include/qextmdichildfrmcaption.h -o qextmdichildfrmcaption.moc

$(srcdir)/qextmdichildview.cpp: qextmdichildview.moc
qextmdichildview.moc: ../include/qextmdichildview.h
	$(MOC) ../include/qextmdichildview.h -o qextmdichildview.moc

$(srcdir)/qextmdimainfrm.cpp: kextmdimainfrm.moc
kextmdimainfrm.moc: ../include/kextmdimainfrm.h
	$(MOC) ../include/kextmdimainfrm.h -o kextmdimainfrm.moc

$(srcdir)/qextmditaskbar.cpp: qextmditaskbar.moc
qextmditaskbar.moc: ../include/qextmditaskbar.h
	$(MOC) ../include/qextmditaskbar.h -o qextmditaskbar.moc


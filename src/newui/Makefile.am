INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets \
	-I$(top_srcdir)/src/profileengine/lib $(all_includes)
METASOURCES = AUTO


lib_LTLIBRARIES = libd.la
libd_la_LDFLAGS = $(all_libraries)

noinst_HEADERS = dmainwindow.h ddockwidget.h docksplitter.h dtabwidget.h
libd_la_SOURCES = dmainwindow.cpp \
	ddockwidget.cpp docksplitter.cpp dtabwidget.cpp
libd_la_LIBADD = $(LIB_KPARTS)

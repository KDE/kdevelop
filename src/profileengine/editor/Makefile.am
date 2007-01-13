INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/external_interfaces \
	-I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets \
	-I$(top_srcdir)/src/profileengine/lib -I$(top_srcdir)/lib/structure $(all_includes)
METASOURCES = AUTO

libprofileeditor_la_LDFLAGS = $(all_libraries)
libprofileeditor_la_LIBADD = $(top_builddir)/src/profileengine/lib/libprofileengine.la
noinst_LTLIBRARIES = libprofileeditor.la
libprofileeditor_COMPILE_FIRST = profileeditorbase.h
libprofileeditor_la_SOURCES = profileeditorbase.ui profileeditor.cpp \
	addprofilewidget.ui
noinst_HEADERS = profileeditor.h


bin_PROGRAMS = kdevprofileeditor
kdevprofileeditor_LDFLAGS = $(all_libraries)
kdevprofileeditor_LDADD = \
	$(top_builddir)/src/profileengine/lib/libprofileengine.la libprofileeditor.la \
	$(LIB_KDEUI) $(LIB_KIO)
kdevprofileeditor_SOURCES = main.cpp



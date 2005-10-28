
METASOURCES = AUTO

bin_PROGRAMS = katehighlight

INCLUDES = -I$(top_srcdir)/test/include $(all_includes)

#INCLUDES = -I$(top_srcdir)/qttestlib -I$(srcdir)/../interfaces \
#	-I$(srcdir)/../part -I$(top_srcdir) -I$(top_srcdir)/kdeprint -I$(top_srcdir)/interfaces \
#	-I$(top_srcdir)/interfaces/kregexpeditor -I$(top_srcdir)/kdefx -I$(top_srcdir)/kutils -I$(top_builddir)/kjs \
#	$(all_includes)

katehighlight_SOURCES = katetest.cpp arbitraryhighlighttest.cpp decoder.cpp \
	cool.cpp cool-lexer.ll editmodelbuilder.cpp coolhighlights.cpp

katehighlight_LDADD = -lktexteditor
katehighlight_LDFLAGS = $(KDE_RPATH) $(all_libraries)
SUBDIRS = include
noinst_HEADERS = editmodelbuilder.h coolhighlights.h

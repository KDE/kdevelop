INCLUDES = $(all_includes) -I$(top_srcdir)/lib/compat $(KDEVMDI_INCLUDES)
METASOURCES = AUTO

noinst_LTLIBRARIES = libwidgets.la

libwidgets_la_SOURCES = ktabzoomwidget.cpp ktabzoomframe.cpp ktabwidget.cpp flagboxes.cpp qcomboview.cpp klistviewaction.cpp
# David claims in his How to write good Makefile.am talk that this isn't legal - I still find it useful :)
libwidgets_la_LIBADD = -lkmdi 
libwidgets_la_LDFLAGS = $(all_libraries)


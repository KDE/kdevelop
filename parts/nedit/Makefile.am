METASOURCES = AUTO

kde_services_DATA = kneditpart.desktop

rcdir = $(kde_datadir)/knedit

lib_LTLIBRARIES = libkneditpart.la

libkneditpart_la_SOURCES = document.cpp view.cpp kneditfactory.cpp
libkneditpart_la_LIBADD   = -lktexteditor
libkneditpart_la_LDFLAGS = $(all_libraries) $(KDE_RPATH)

INCLUDES= -I$(top_srcdir)/interfaces -I$(top_srcdir)/kparts -I$(top_srcdir) $(all_includes)

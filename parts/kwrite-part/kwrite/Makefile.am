INCLUDES = -I$(top_srcdir)/lib/util $(all_includes)

KDE_CXXFLAGS = $(USE_RTTI) -DNO_NEW_CODE

noinst_LTLIBRARIES = libkdevkwrite.la

libkdevkwrite_la_SOURCES = highlight.cpp \
			   kwdoc.cpp kwview.cpp undohistory.cpp  \
			   kwdialog.cpp \
			   kwbuffer.cpp kwtextline.cpp kvmallocator.cpp \
			   kwattribute.cpp syntaxdocument.cpp

METASOURCES = AUTO

syntaxdir = $(kde_datadir)/kwriteeditor_part
syntax_DATA = syntax.xml

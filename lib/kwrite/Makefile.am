INCLUDES = $(all_includes) -I$(top_srcdir)/lib/util

KDE_CXXFLAGS = $(USE_RTTI) -DNO_NEW_CODE

noinst_LTLIBRARIES = libkdevkwrite.la

libkdevkwrite_la_SOURCES = highlight.cpp texteditor.cpp \
			   kwdoc.cpp kwview.cpp undohistory.cpp  \
			   kwdialog.cpp kwrite_factory.cpp \
			   kwbuffer.cpp kwtextline.cpp kvmallocator.cpp \
			   kwattribute.cpp kwriteIface.skel syntaxdocument.cpp \
			   editorpart.cpp

METASOURCES = AUTO

EXTRA_DIST = AUTHORS

syntaxdir = $(kde_datadir)/kwrite
syntax_DATA = syntax.xml







KDE_CXXFLAGS=-UQT_NO_COMPAT
INCLUDES=-I$(top_srcdir) $(all_includes)
noinst_LTLIBRARIES = libqt3stuff.la libqt3widgets.la
libqt3stuff_la_SOURCES = qcomplextext.cpp qrichtext.cpp qstylesheet.cpp mocstuff.cpp
libqt3stuff_la_LDFLAGS = $(all_libraries) -no-undefined
libqt3stuff_la_LIBADD = $(LIB_QT)
libqt3stuff_la_METASOURCES = qrichtext_p.moc qstylesheet.moc

libqt3widgets_la_SOURCES = qtextedit.cpp qtextview.cpp mocwidgets.cpp
libqt3widgets_la_LDFLAGS = $(all_libraries) -no-undefined
libqt3widgets_la_LIBADD = libqt3stuff.la $(LIB_QT)
libqt3widgets_la_METASOURCES = qtextedit.moc qtextview.moc
KDE_OPTIONS = qtonly

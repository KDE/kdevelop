CXXFLAGS=$(SHORTCXXFLAGS)
INCLUDES = -I$(top_srcdir)/interfaces $(all_includes)
METASOURCES = AUTO
kdevdesignerdir = $(kde_moduledir)/plugins/kdevdesigner
kdevdesigner_LTLIBRARIES =  libkdevdesigner_lang.la
libkdevdesigner_lang_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
libkdevdesigner_lang_la_SOURCES =  languageinterfaceimpl.cpp languageinterfaceimpl.h
libkdevdesigner_lang_la_LIBADD = $(LIB_QT)

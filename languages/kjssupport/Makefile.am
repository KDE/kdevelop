INCLUDES = -I$(kde_includes)/kdevelop $(all_includes)

kde_module_LTLIBRARIES = libkdevkjssupport.la
libkdevkjssupport_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevkjssupport_la_LIBADD = -lkjsembed -lkdevelop
libkdevkjssupport_la_SOURCES = kjssupport_part.cpp kjsproblems.cpp jscodecompletion.cpp subclassingdlg.cpp subclassingdlgbase.ui

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevkjssupport.desktop

mimesrcdir = $(kde_mimedir)/text
mimesrc_DATA = x-javascript-source.desktop

EXTRA_DIST = $(mimesrc_DATA)

rcdir = $(kde_datadir)/kdevkjssupport
rc_DATA = kdevkjssupport.rc

SUBDIRS = template subclassing_template
noinst_HEADERS =  kjsproblems.h

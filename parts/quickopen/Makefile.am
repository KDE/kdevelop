INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets $(all_includes)

kde_module_LTLIBRARIES = libkdevquickopen.la
libkdevquickopen_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN)
libkdevquickopen_la_LIBADD = $(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/widgets/libwidgets.la

libkdevquickopen_la_SOURCES = quickopen_part.cpp quickopendialog.cpp quickopenclassdialog.cpp quickopenfunctionchooseform.cpp quickopenbase.ui quickopenfiledialog.cpp quickopenfiledialog.h quickopenfunctiondialog.cpp quickopenfunctiondialog.h

METASOURCES = AUTO

servicedir = $(kde_servicesdir)
service_DATA = kdevpart_quickopen.desktop

rcdir = $(kde_datadir)/kdevquickopen
rc_DATA = kdevpart_quickopen.rc
noinst_HEADERS = quickopendialog.h quickopenfunctionchooseform.h quickopendialog.h

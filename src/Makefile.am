INCLUDES = -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib $(KDEVMDI_INCLUDES) -I$(top_srcdir)/lib/structure $(all_includes)

bin_PROGRAMS = kdevelop

kdevelop_SOURCES = main.cpp core.cpp partcontroller.cpp projectmanager.cpp plugincontroller.cpp api.cpp partselectwidget.cpp settingswidget.ui toplevel.cpp statusbar.cpp editorproxy.cpp debugger.cpp documentationpart.cpp splashscreen.cpp projectsession.cpp mainwindowshare.cpp generalinfowidgetbase.ui generalinfowidget.cpp languageselectwidget.cpp newmainwindow.cpp

kdevelop_METASOURCES = AUTO
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdevelop_LDADD = $(top_builddir)/lib/widgets/libwidgets.la $(LIB_KDEUI) $(LIB_KPARTS) $(LIB_KHTML) -lktexteditor $(LIB_KDEVMDI) $(top_builddir)/lib/libkdevelop.la

rcdir = $(kde_datadir)/kdevelop
rc_DATA = kdevelopui.rc eventsrc documentation_partui.rc

# default KDevelop configuration
kdevelopdatadir = $(kde_confdir)
kdevelopdata_DATA = kdeveloprc

SUBDIRS = profiles
noinst_HEADERS = languageselectwidget.h newmainwindow.h

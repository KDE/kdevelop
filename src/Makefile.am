INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/interfaces -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets $(KDEVMDI_INCLUDES) -I$(top_srcdir)/lib/structure $(all_includes)

bin_PROGRAMS = kdevelop kdevassistant

kdevelop_SOURCES = main.cpp core.cpp partcontroller.cpp projectmanager.cpp plugincontroller.cpp api.cpp partselectwidget.cpp settingswidget.ui toplevel.cpp statusbar.cpp editorproxy.cpp debugger.cpp documentationpart.cpp splashscreen.cpp projectsession.cpp mainwindowshare.cpp generalinfowidgetbase.ui generalinfowidget.cpp languageselectwidget.cpp newmainwindow.cpp mimewarningdialog.ui

kdevelop_METASOURCES = AUTO
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdevelop_LDADD = $(top_builddir)/lib/widgets/libwidgets.la $(LIB_KDEUI) $(LIB_KPARTS) $(LIB_KHTML) -lktexteditor $(LIB_KDEVMDI) $(top_builddir)/lib/libkdevelop.la

rcdir = $(kde_datadir)/kdevelop
rc_DATA = kdevelopui.rc eventsrc

# default KDevelop configuration
kdevelopdatadir = $(kde_confdir)
kdevelopdata_DATA = kdeveloprc

SUBDIRS = profiles
noinst_HEADERS = languageselectwidget.h newmainwindow.h
kdevassistant_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdevassistant_SOURCES = api.cpp core.cpp debugger.cpp documentationpart.cpp editorproxy.cpp generalinfowidget.cpp languageselectwidget.cpp mainwindowshare.cpp newmainwindow.cpp partcontroller.cpp partselectwidget.cpp plugincontroller.cpp projectmanager.cpp projectsession.cpp settingswidget.cpp splashscreen.cpp statusbar.cpp toplevel.cpp generalinfowidgetbase.ui main_assistant.cpp mimewarningdialog.ui
kdevassistant_LDADD = $(top_builddir)/lib/widgets/libwidgets.la $(top_builddir)/lib/libkdevelop.la -lktexteditor $(LIB_KHTML) $(LIB_KDEVMDI) $(LIB_KPARTS) $(LIB_KDEUI)
rc_assistantdir = $(kde_datadir)/kdevassistant
rc_assistant_DATA =  kdevassistantui.rc

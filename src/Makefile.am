INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets \
	$(KDEVMDI_INCLUDES) $(all_includes)

bin_PROGRAMS = kdevelop kdevassistant

kdevelop_SOURCES = core.cpp partcontroller.cpp projectmanager.cpp \
	plugincontroller.cpp api.cpp partselectwidget.cpp settingswidget.ui toplevel.cpp \
	statusbar.cpp editorproxy.cpp documentationpart.cpp projectsession.cpp \
	mainwindowshare.cpp generalinfowidgetbase.ui generalinfowidget.cpp languageselectwidget.cpp \
	newmainwindow.cpp mimewarningdialog.ui main.cpp splashscreen.cpp

kdevelop_METASOURCES = AUTO
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdevelop_LDADD = $(top_builddir)/lib/widgets/libkdevwidgets.la \
	$(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la $(LIB_KDEVMDI) \
	$(LIB_KDEUI) $(LIB_KPARTS) $(LIB_KHTML) -lktexteditor

rcdir = $(kde_datadir)/kdevelop
rc_DATA = kdevelopui.rc eventsrc

# default KDevelop configuration
kdevelopdatadir = $(kde_confdir)
kdevelopdata_DATA = kdeveloprc kdevpluginprofilerc

SUBDIRS = profiles profileengine
noinst_HEADERS = languageselectwidget.h newmainwindow.h
kdevassistant_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdevassistant_SOURCES = api.cpp core.cpp documentationpart.cpp editorproxy.cpp \
	generalinfowidget.cpp languageselectwidget.cpp mainwindowshare.cpp newmainwindow.cpp \
	partcontroller.cpp partselectwidget.cpp plugincontroller.cpp projectmanager.cpp \
	projectsession.cpp settingswidget.ui statusbar.cpp toplevel.cpp generalinfowidgetbase.ui \
	main_assistant.cpp mimewarningdialog.ui splashscreen.cpp
kdevassistant_LDADD = $(top_builddir)/lib/widgets/libkdevwidgets.la \
	$(top_builddir)/lib/libkdevelop.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la $(LIB_KDEVMDI) \
	-lktexteditor $(LIB_KHTML) $(LIB_KPARTS) $(LIB_KDEUI)

rc_assistantdir = $(kde_datadir)/kdevassistant
rc_assistant_DATA =  kdevassistantui.rc

KDE_OPTIONS=nofinal

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets \
	-I$(top_srcdir)/src/profileengine/lib $(KDEVMDI_INCLUDES) $(all_includes)

KDE_OPTIONS=nofinal

lib_LTLIBRARIES = libkdevshell.la
libkdevshell_la_LDFLAGS = $(all_libraries)
libkdevshell_la_LIBADD = \
	$(top_builddir)/src/profileengine/lib/libprofileengine.la $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/widgets/libkdevwidgets.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la \
        $(LIB_KDEVMDI) $(LIB_KDEUI) $(LIB_KPARTS) $(LIB_KHTML) -lktexteditor
libkdevshell_la_SOURCES = api.cpp core.cpp documentationpart.cpp \
	editorproxy.cpp generalinfowidget.cpp languageselectwidget.cpp mainwindowshare.cpp \
	newmainwindow.cpp partcontroller.cpp partselectwidget.cpp plugincontroller.cpp \
	projectmanager.cpp projectsession.cpp splashscreen.cpp statusbar.cpp toplevel.cpp \
	generalinfowidgetbase.ui mimewarningdialog.ui shellextension.cpp

bin_PROGRAMS = kdevelop kdevassistant

kdevelop_SOURCES = main.cpp kdevideextension.cpp settingswidget.ui

kdevelop_METASOURCES = AUTO
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdevelop_LDADD = $(top_builddir)/src/libkdevshell.la

rcdir = $(kde_datadir)/kdevelop
rc_DATA = kdevelopui.rc eventsrc

# default KDevelop configuration
kdevelopdatadir = $(kde_confdir)
kdevelopdata_DATA = kdeveloprc

SUBDIRS = profiles profileengine
kdevassistant_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdevassistant_SOURCES = main_assistant.cpp kdevassistantextension.cpp
kdevassistant_LDADD = $(top_builddir)/src/libkdevshell.la

rc_assistantdir = $(kde_datadir)/kdevassistant
rc_assistant_DATA =  kdevassistantui.rc
shellincludedir = $(includedir)/kdevelop/shell
shellinclude_HEADERS = api.h core.h documentationpart.h editorproxy.h\
	generalinfowidget.h languageselectwidget.h mainwindowshare.h\
	newmainwindow.h partcontroller.h partselectwidget.h plugincontroller.h\
	projectmanager.h projectsession.h settingswidget.ui.h shellextension.h\
	splashscreen.h statusbar.h toplevel.h generalinfowidgetbase.h\
	mimewarningdialog.h settingswidget.h

INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/lib/interfaces \
	-I$(top_srcdir)/lib/interfaces/extensions -I$(top_srcdir)/lib/interfaces/external \
	-I$(top_srcdir)/lib/interfaces/extras -I$(top_srcdir)/lib/util -I$(top_srcdir)/lib/widgets \
	-I$(top_srcdir)/src/newui -I$(top_srcdir)/src/profileengine/lib $(KDEVMDI_INCLUDES) \
	$(all_includes)

KDE_OPTIONS=nofinal

lib_LTLIBRARIES = libkdevshell.la
libkdevshell_la_LDFLAGS = $(all_libraries)
libkdevshell_la_LIBADD = \
	$(top_builddir)/src/profileengine/lib/libprofileengine.la $(top_builddir)/lib/libkdevelop.la \
	$(top_builddir)/lib/widgets/libkdevwidgets.la $(top_builddir)/lib/interfaces/extras/libkdevextras.la \
	$(top_builddir)/src/newui/libd.la $(LIB_KDEVMDI) $(LIB_KHTML) -lktexteditor
libkdevshell_la_SOURCES = api.cpp core.cpp documentationpart.cpp \
	editorproxy.cpp generalinfowidget.cpp languageselectwidget.cpp mainwindowshare.cpp \
	newmainwindow.cpp partcontroller.cpp plugincontroller.cpp projectmanager.cpp \
	projectsession.cpp splashscreen.cpp statusbar.cpp toplevel.cpp generalinfowidgetbase.ui \
	mimewarningdialog.ui shellextension.cpp simplemainwindow.cpp multibuffer.cpp \
	pluginselectdialogbase.ui pluginselectdialog.cpp pluginselectdialog.h

bin_PROGRAMS = kdevelop kdevassistant

kdevelop_SOURCES = main.cpp kdevideextension.cpp settingswidget.ui

kdevelop_METASOURCES = AUTO
kdevelop_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdevelop_LDADD = libkdevshell.la

rcdir = $(kde_datadir)/kdevelop
rc_DATA = kdevelopui.rc eventsrc

# default KDevelop configuration
kdevelopdatadir = $(kde_confdir)
kdevelopdata_DATA = kdeveloprc kdevassistantrc

SUBDIRS = profiles profileengine newui kconf_update
kdevassistant_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdevassistant_SOURCES = main_assistant.cpp kdevassistantextension.cpp
kdevassistant_LDADD = libkdevshell.la

rc_assistantdir = $(kde_datadir)/kdevassistant
rc_assistant_DATA =  kdevassistantui.rc
shellincludedir = $(includedir)/kdevelop/shell
shellinclude_HEADERS = api.h core.h documentationpart.h editorproxy.h \
	generalinfowidget.h languageselectwidget.h mainwindowshare.h newmainwindow.h \
	partcontroller.h plugincontroller.h projectmanager.h projectsession.h shellextension.h \
	splashscreen.h statusbar.h toplevel.h generalinfowidgetbase.h mimewarningdialog.h \
	settingswidget.h simplemainwindow.h multibuffer.h

DOXYGEN_REFERENCES = dcop interfaces kdecore kdefx kdeui khtml kmdi kio kjs kparts kutils kdevutil kdevinterfaces kdevextensions
DOXYGEN_PROJECTNAME = KDevelop Generic Shell
DOXYGEN_DOCDIRPREFIX = kdevshell
include ../Doxyfile.am
profilesdatadir = $(kde_datadir)/kdevelop/profiles
profilesdata_DATA = projectprofiles
AM_CXXFLAGS = -w

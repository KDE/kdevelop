INCLUDES = -I$(top_srcdir)/lib/interfaces \
	   -I$(top_srcdir)/lib/sourceinfo \
	   -I$(top_srcdir)/lib/util \
	   -I$(top_srcdir)/lib \
	   -I$(top_srcdir)/lib/qextmdi/include \
	   $(all_includes)

bin_PROGRAMS = gideon

gideon_SOURCES = main.cpp toplevel_mdi.cpp core.cpp partcontroller.cpp \
		 projectmanager.cpp plugincontroller.cpp api.cpp \
		 partselectwidget.cpp settingswidget.ui toplevel.cpp \
		 toplevel_sdi.cpp statusbar.cpp editorproxy.cpp \
		 debugger.cpp documentationpart.cpp splashscreen.cpp

gideon_METASOURCES = AUTO
gideon_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
gideon_LDADD = $(top_builddir)/lib/widgets/libwidgets.la \
	       	$(LIB_KDEUI) $(LIB_KPARTS) $(LIB_KHTML) \
		-lktexteditor \
		$(top_builddir)/lib/interfaces/libkdevinterfaces.la \
		$(top_builddir)/lib/qextmdi/src/libkdevelopqextmdi.la \
		$(top_builddir)/lib/libkdevelop.la

rcdir = $(kde_datadir)/gideon
rc_DATA = gideonui.rc

# default KDevelop	 configuration
#datadir =$(kde_confdir)
#data_DATA =gideonrc


INCLUDES = -I$(top_srcdir)/lib/interfaces \
	   -I$(top_srcdir)/lib/sourceinfo \
	   -I$(top_srcdir)/lib/util \
	   -I$(top_srcdir)/lib \
	   $(all_includes)

bin_PROGRAMS = gideon

gideon_SOURCES = main.cpp toplevel.cpp core.cpp partcontroller.cpp \
		 projectmanager.cpp plugincontroller.cpp api.cpp \
		 partselectwidget.cpp settingswidget.ui

gideon_METASOURCES = AUTO
gideon_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
gideon_LDADD = $(top_builddir)/lib/widgets/libwidgets.la \
	       	$(LIB_KDEUI) $(LIB_KPARTS) \
		-lktexteditor \
		$(top_builddir)/lib/interfaces/libkdevinterfaces.la \
		$(top_builddir)/lib/libkdevelop.la

#$(top_builddir)/parts/doctreeview/libkdevdoctreeview.la \
#	       $(top_builddir)/parts/classview/libkdevclassview.la \
#	       $(top_builddir)/parts/grepview/libkdevgrepview.la \
#	       $(top_builddir)/parts/outputviews/libkdevoutputviews.la \
#	       $(top_builddir)/parts/appwizard/libkdevappwizard.la \
#	       $(top_builddir)/lib/util/libkdevutil.la \
#	       $(LIB_KHTML)

rcdir = $(kde_datadir)/gideon
rc_DATA = gideonui.rc

# default KDevelop	 configuration
#datadir =$(kde_confdir)
#data_DATA =gideonrc


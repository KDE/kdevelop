INCLUDES = -I$(top_srcdir)/lib/interfaces \
	   -I$(top_srcdir)/lib/sourceinfo \
	   -I$(top_srcdir)/lib/kwrite \
	   -I$(top_srcdir)/lib/util \
	   $(all_includes)

bin_PROGRAMS = gideon

gideon_SOURCES = main.cpp core.cpp toplevel.cpp partloader.cpp documentationpart.cpp \
		 importdlg.cpp partselectwidget.cpp filenameedit.cpp bufferaction.cpp \
		 statusbar.cpp 
gideon_METASOURCES = AUTO
gideon_LDFLAGS = $(all_libraries) $(KDE_RPATH) 
gideon_LDADD = $(top_builddir)/parts/doctreeview/libkdevdoctreeview.la \
	       $(top_builddir)/parts/classview/libkdevclassview.la \
	       $(top_builddir)/parts/grepview/libkdevgrepview.la \
	       $(top_builddir)/parts/outputviews/libkdevoutputviews.la \
	       $(top_builddir)/parts/appwizard/libkdevappwizard.la \
	       $(top_builddir)/lib/libkdevelop.la \
		   $(top_builddir)/lib/interfaces/libkdevinterfaces.la \
	       $(top_builddir)/lib/util/libkdevutil.la \
		   $(top_builddir)/lib/interfaces/keditor/libkeditorinterface.la \
	       $(LIB_KHTML)

rcdir = $(kde_datadir)/gideon
rc_DATA = gideonui.rc


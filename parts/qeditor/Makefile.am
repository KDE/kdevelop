# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
#SUBDIRS = .

# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# these are the headers for your project
noinst_HEADERS = 

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/qeditor.pot

kde_module_LTLIBRARIES = libqeditorpart.la

# the Part's source, library search path, and link libraries
libqeditorpart_la_SOURCES = cpp_colorizer.cpp	 qeditor_view.cpp 	java_colorizer.cpp	 qeditorcodecompletion_arghint.cpp 	parenmatcher.cpp	 qmake_colorizer.cpp 	python_colorizer.cpp	 qsourcecolorizer.cpp 	qeditor.cpp		 simple_indent.cpp 	qeditor_meta_unload.cpp  xml_colorizer.cpp 	qeditor_part.cpp	 qeditorcodecompletion_iface_impl.cpp 	paragdata.cpp linenumberwidget.h linenumberwidget.cpp markerwidget.cpp gotolinedialog.ui settingsdialog.ui koFind.cpp koReplace.cpp levelwidget.cpp cindent.cpp yyindent.cpp

libqeditorpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
libqeditorpart_la_LIBADD  = $(LIB_KPARTS) $(LIB_KFILE) -lktexteditor

# this is where the desktop file will go 
partdesktopdir   = $(kde_servicesdir)
partdesktop_DATA = qeditor_part.desktop

# this is where the part's XML-GUI resource file goes
partrcdir   = $(kde_datadir)/qeditorpart
partrc_DATA = qeditor_part.rc

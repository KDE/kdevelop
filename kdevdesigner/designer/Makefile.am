SUBDIRS = pics

CXXFLAGS=$(SHORTCXXFLAGS) -DDESIGNER
INCLUDES = -I$(top_srcdir)/kdevdesigner/shared -I$(top_srcdir)/lib/external_interfaces $(all_includes)
METASOURCES = AUTO
kde_module_LTLIBRARIES =  libkdevdesignerpart.la
libkdevdesignerpart_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries)
libkdevdesignerpart_la_LIBADD = $(top_builddir)/kdevdesigner/uilib/libkdevqui.la $(top_builddir)/kdevdesigner/shared/libshared.la $(top_builddir)/lib/external_interfaces/libkinterfacedesigner.la -lqassistantclient $(LIB_KFILE) $(LIB_KPARTS)
libkdevdesignerpart_la_SOURCES = kdevdesigner_part.cpp actiondnd.cpp actioneditorimpl.cpp actionlistview.cpp asciivalidator.cpp command.cpp connectionitems.cpp connectiontable.cpp customwidgeteditorimpl.cpp database.cpp dbconnectionimpl.cpp dbconnectionsimpl.cpp defs.cpp designerapp.cpp designerappiface.cpp editfunctionsimpl.cpp filechooser.cpp formfile.cpp formsettingsimpl.cpp formwindow.cpp hierarchyview.cpp iconvieweditorimpl.cpp layout.cpp listboxdnd.cpp listboxeditorimpl.cpp listboxrename.cpp listdnd.cpp listviewdnd.cpp listvieweditorimpl.cpp mainwindow.cpp mainwindowactions.cpp menubareditor.cpp metadatabase.cpp multilineeditorimpl.cpp newformimpl.cpp orderindicator.cpp outputwindow.cpp paletteeditoradvancedimpl.cpp paletteeditorimpl.cpp pixmapchooser.cpp pixmapcollection.cpp popupmenueditor.cpp previewframe.cpp previewwidgetimpl.cpp project.cpp projectsettingsimpl.cpp propertyeditor.cpp propertyobject.cpp qcompletionedit.cpp resource.cpp sizehandle.cpp sourceeditor.cpp sourcefile.cpp startdialogimpl.cpp styledbutton.cpp syntaxhighlighter_html.cpp tableeditorimpl.cpp timestamp.cpp variabledialogimpl.cpp widgetaction.cpp widgetfactory.cpp wizardeditorimpl.cpp workspace.cpp about.ui dbconnection.ui gotolinedialog.ui newform.ui previewwidget.ui variabledialog.ui actioneditor.ui dbconnectioneditor.ui iconvieweditor.ui paletteeditor.ui projectsettings.ui wizardeditor.ui configtoolboxdialog.ui dbconnections.ui listboxeditor.ui paletteeditoradvanced.ui replacedialog.ui connectiondialog.ui editfunctions.ui listeditor.ui pixmapcollectioneditor.ui richtextfontdialog.ui createtemplate.ui finddialog.ui listvieweditor.ui pixmapfunction.ui startdialog.ui customwidgeteditor.ui formsettings.ui multilineeditor.ui preferences.ui tableeditor.ui designeraction.cpp myiconloader.cpp
partrcdir = $(kde_datadir)/kdevdesignerpart
partdesktopdir = $(kde_servicesdir)
partdesktop_DATA =  kdevdesigner_part.desktop
partrc_DATA = kdevdesigner_part.rc kdevdesigner_part_sh.rc
noinst_HEADERS = kdevdesigner_part.h designeraction.h myiconloader.h
picsdirdir = $(kde_datadir)/kdevdesignerpart/pics

KDE_OPTIONS=nofinal
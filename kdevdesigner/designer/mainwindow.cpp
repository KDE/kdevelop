 /**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "designerappiface.h"
#include "designerapp.h"

#include "mainwindow.h"
#include "defs.h"
#include "globaldefs.h"
#include "formwindow.h"
#include "widgetdatabase.h"
#include "widgetfactory.h"
#include "propertyeditor.h"
#include "metadatabase.h"
#include "resource.h"
#include "hierarchyview.h"
#include "newformimpl.h"
#include "workspace.h"
#include "about.h"
#include "multilineeditorimpl.h"
#include "wizardeditorimpl.h"
#include "outputwindow.h"
#include "actioneditorimpl.h"
#include "actiondnd.h"
#include "project.h"
#include "projectsettingsimpl.h"
#include "qwidgetfactory.h"
#include "pixmapcollection.h"
#include "qcompletionedit.h"
#include "sourcefile.h"
#include "orderindicator.h"
#include <qtoolbox.h>
#include "widgetaction.h"
#include "propertyobject.h"
#include "popupmenueditor.h"
#include "menubareditor.h"

#include "startdialog.h"
#include "createtemplate.h"
#include "editfunctions.h"
//#include "connectionviewer.h"
#include "formsettings.h"
#include "preferences.h"
#include "pixmapcollectioneditor.h"
#ifndef QT_NO_SQL
#include "dbconnectionimpl.h"
#endif
//#include "connectioneditor.h"
#include "customwidgeteditorimpl.h"
#include "paletteeditorimpl.h"
#include "listboxeditorimpl.h"
#include "listvieweditorimpl.h"
#include "iconvieweditorimpl.h"
#include "tableeditorimpl.h"
#include "multilineeditor.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "gotolinedialog.h"
#include "designeraction.h"

#include <kiconloader.h>

#include <qinputdialog.h>
#include <qtoolbar.h>
#include <qfeatures.h>
#include <qmetaobject.h>
#include <qaction.h>
#include <qpixmap.h>
#include <qworkspace.h>
#include <qfiledialog.h>
#include <qclipboard.h>
#include <qmessagebox.h>
#include <qbuffer.h>
#include <qdir.h>
#include <qstyle.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qfile.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qwizard.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <qdockwindow.h>
#include <qstylefactory.h>
#include <qvbox.h>
#include <qprocess.h>
#include <qsettings.h>
#include <qaccel.h>
#include <qtooltip.h>
#include <qsizegrip.h>
#include <qtextview.h>
#include <qassistantclient.h>
#include <stdlib.h>

#include "kdevdesigner_part.h"

static bool mblockNewForms = FALSE;
extern QMap<QWidget*, QString> *qwf_forms;
extern QString *qwf_language;
extern bool qwf_execute_code;
extern bool qwf_stays_on_top;
extern void set_splash_status( const QString &txt );
extern QObject* qwf_form_object;
extern QString *qwf_plugin_dir;

MainWindow *MainWindow::self = 0;

QString assistantPath()
{
    QString path = QDir::cleanDirPath( QString( qInstallPathBins() ) +
				       QDir::separator() );
    return path;
}


static QString textNoAccel( const QString& text)
{
    QString t = text;
    int i;
    while ( (i = t.find('&') )>= 0 ) {
	t.remove(i,1);
    }
    return t;
}


MainWindow::MainWindow( KDevDesignerPart *part, bool asClient, bool single, const QString &plgDir )
    : QMainWindow( 0, "designer_mainwindow"/*, WType_TopLevel | (single ? 0 : WDestructiveClose) | WGroupLeader*/ ),
      grd( 10, 10 ), sGrid( TRUE ), snGrid( TRUE ), restoreConfig( TRUE ), splashScreen( TRUE ),
      fileFilter( tr( "Qt User-Interface Files (*.ui)" ) ), client( asClient ),
      previewing( FALSE ), databaseAutoEdit( FALSE ), autoSaveEnabled( FALSE ), autoSaveInterval( 1800 ), m_part(part)
{
//    extern void qInitImages_designercore();
//    qInitImages_designercore();

    self = this;
    setPluginDirectory( plgDir );
    customWidgetToolBar = customWidgetToolBar2 = 0;
    guiStuffVisible = TRUE;
    editorsReadOnly = FALSE;
    sSignalHandlers = TRUE;
    init_colors();
    shStartDialog = FALSE;

    desInterface = new DesignerInterfaceImpl( this );
    desInterface->addRef();
    inDebugMode = FALSE;
    savePluginPaths = FALSE;

    updateFunctionsTimer = new QTimer( this );
    connect( updateFunctionsTimer, SIGNAL( timeout() ),
	     this, SLOT( doFunctionsChanged() ) );

    autoSaveTimer = new QTimer( this );
    connect( autoSaveTimer, SIGNAL( timeout() ),
	     this, SLOT( fileSaveAll() ) );

    set_splash_status( "Loading Plugins..." );
    setupPluginManagers();

/*    if ( !single )
	qApp->setMainWidget( this );*/
    QWidgetFactory::addWidgetFactory( new CustomWidgetFactory );
#ifndef Q_WS_MACX
    setIcon( BarIcon( "designer_appicon.png", KDevDesignerPartFactory::instance() ) );
#endif

    actionGroupTools = 0;
    prefDia = 0;
    windowMenu = 0;
    hierarchyView = 0;
    actionEditor = 0;
    currentProject = 0;
    wspace = 0;
    oWindow = 0;
    singleProject = single;

    statusBar()->clear();
#if defined(QT_NON_COMMERCIAL)
    statusBar()->addWidget( new QLabel(tr("Ready - This is the non-commercial version of Qt - "
	"For commercial evaluations, use the help menu to register with Trolltech."), statusBar()), 1 );
#else
    statusBar()->addWidget( new QLabel("Ready", statusBar()), 1 );
#endif


    set_splash_status( "Setting up GUI..." );
    setupMDI();
    setupMenuBar();

    setupToolbox();

    setupFileActions();
    setupEditActions();
    setupProjectActions();
    setupSearchActions();
    layoutToolBar = new QToolBar( this, "Layout" );
    layoutToolBar->setCloseMode( QDockWindow::Undocked );
    addToolBar( layoutToolBar, tr( "Layout" ) );
    interpreterPluginManager = 0;
    setupToolActions();
    setupLayoutActions();
    setupPreviewActions();
    setupOutputWindow();
    setupActionManager();
    setupWindowActions();

    setupWorkspace();
    setupHierarchyView();
    setupPropertyEditor();
    setupActionEditor();

    setupHelpActions();

    setupRMBMenus();

    connect( this, SIGNAL( projectChanged() ), this, SLOT( emitProjectSignals() ) );
    connect( this, SIGNAL( hasActiveWindow(bool) ), this, SLOT( emitProjectSignals() ) );

    emit hasActiveForm( FALSE );
    emit hasActiveWindow( FALSE );

    lastPressWidget = 0;
    qApp->installEventFilter( this );

    QSize as( qApp->desktop()->size() );
    as -= QSize( 30, 30 );
    resize( QSize( 1200, 1000 ).boundedTo( as ) );

    connect( qApp->clipboard(), SIGNAL( dataChanged() ),
	     this, SLOT( clipboardChanged() ) );
    clipboardChanged();
    layoutChilds = FALSE;
    layoutSelected = FALSE;
    breakLayout = FALSE;
    backPix = TRUE;

    set_splash_status( "Loading User Settings..." );
    readConfig();
    // hack to make WidgetFactory happy (so it knows QWidget and QDialog for resetting properties)
    QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QWidget" ), this, 0, FALSE );
    delete w;
    w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QDialog" ), this, 0, FALSE );
    delete w;
    w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QLabel" ), this, 0, FALSE );
    delete w;
    w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QTabWidget" ), this, 0, FALSE );
    delete w;
    w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QFrame" ), this, 0, FALSE );
    delete w;
    setAppropriate( (QDockWindow*)actionEditor->parentWidget(), FALSE );
    actionEditor->parentWidget()->hide();

    assistant = new QAssistantClient( assistantPath(), this );

    statusBar()->setSizeGripEnabled( TRUE );
    set_splash_status( "Initialization Done." );
/*    if ( shStartDialog )
	QTimer::singleShot( 0, this, SLOT( showStartDialog() ));*/

    if ( autoSaveEnabled )
	autoSaveTimer->start( autoSaveInterval * 1000 );
    
    setFocusPolicy( ClickFocus );
}

MainWindow::~MainWindow()
{
    QValueList<Tab>::Iterator tit;
    for ( tit = preferenceTabs.begin(); tit != preferenceTabs.end(); ++tit ) {
	Tab t = *tit;
	delete t.w;
    }
    for ( tit = projectTabs.begin(); tit != projectTabs.end(); ++tit ) {
	Tab t = *tit;
	delete t.w;
    }

    QMap< QAction*, Project* >::Iterator it = projects.begin();
    while ( it != projects.end() ) {
	Project *p = *it;
	++it;
	delete p;
    }
    projects.clear();

    delete oWindow;
    oWindow = 0;

    desInterface->release();
    desInterface = 0;

    delete actionPluginManager;
    delete preferencePluginManager;
    delete projectSettingsPluginManager;
    delete interpreterPluginManager;
    delete templateWizardPluginManager;
    delete editorPluginManager;
    delete sourceTemplatePluginManager;

    MetaDataBase::clearDataBase();
    if(self == this)
	self = 0;
}

void MainWindow::setupMDI()
{
    QVBox *vbox = new QVBox( this );
    setCentralWidget( vbox );
    vbox->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    vbox->setMargin( 1 );
    vbox->setLineWidth( 1 );
    qworkspace = new QWorkspace( vbox );
    qworkspace->setPaletteBackgroundPixmap( UserIcon( "designer_background.png", KDevDesignerPartFactory::instance() ) );
    qworkspace->setScrollBarsEnabled( TRUE );
    connect( qworkspace, SIGNAL( windowActivated( QWidget * ) ),
	     this, SLOT( activeWindowChanged( QWidget * ) ) );
    lastActiveFormWindow = 0;
    qworkspace->setAcceptDrops( TRUE );
}

void MainWindow::setupMenuBar()
{
    menubar = menuBar();
}

void MainWindow::setupPropertyEditor()
{
    QDockWindow *dw = new QDockWindow( QDockWindow::InDock, this );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    propertyEditor = new PropertyEditor( dw );
    addToolBar( dw, Qt::DockRight );
    dw->setWidget( propertyEditor );
    dw->setFixedExtentWidth( 250 );
    dw->setCaption( tr( "Property Editor/Signal Handlers" ) );
    QWhatsThis::add( propertyEditor,
		     tr("<b>The Property Editor</b>"
			"<p>You can change the appearance and behavior of the selected widget in the "
			"property editor.</p>"
			"<p>You can set properties for components and forms at design time and see the "
			"immediately see the effects of the changes. "
			"Each property has its own editor which (depending on the property) can be used "
			"to enter "
			"new values, open a special dialog, or to select values from a predefined list. "
			"Click <b>F1</b> to get detailed help for the selected property.</p>"
			"<p>You can resize the columns of the editor by dragging the separators in the "
			"list's header.</p>"
			"<p><b>Signal Handlers</b></p>"
			"<p>In the Signal Handlers tab you can define connections between "
			"the signals emitted by widgets and the slots in the form. "
			"(These connections can also be made using the connection tool.)" ) );
    dw->show();
}

void MainWindow::setupOutputWindow()
{
    QDockWindow *dw = new QDockWindow( QDockWindow::InDock, this );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    addToolBar( dw, Qt::DockBottom );
    oWindow = new OutputWindow( dw );
    dw->setWidget( oWindow );
    dw->setFixedExtentHeight( 150 );
    dw->setCaption( tr( "Output Window" ) );
}

void MainWindow::setupHierarchyView()
{
    if ( hierarchyView )
	return;
    QDockWindow *dw = new QDockWindow( QDockWindow::InDock, this );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    hierarchyView = new HierarchyView( dw );
    addToolBar( dw, Qt::DockRight );
    dw->setWidget( hierarchyView );

    dw->setCaption( tr( "Object Explorer" ) );
    dw->setFixedExtentWidth( 250 );
    QWhatsThis::add( hierarchyView,
		     tr("<b>The Object Explorer</b>"
			"<p>The Object Explorer provides an overview of the relationships "
			"between the widgets in a form. You can use the clipboard functions using "
			"a context menu for each item in the view. It is also useful for selecting widgets "
			"in forms that have complex layouts.</p>"
			"<p>The columns can be resized by dragging the separator in the list's header.</p>"
			"<p>The second tab shows all the form's slots, class variables, includes, etc.</p>") );
    dw->show();
}

void MainWindow::setupWorkspace()
{
    QDockWindow *dw = new QDockWindow( QDockWindow::InDock, this );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    QVBox *vbox = new QVBox( dw );
    QCompletionEdit *edit = new QCompletionEdit( vbox );
    QToolTip::add( edit, tr( "Start typing the buffer you want to switch to here (ALT+B)" ) );
    QAccel *a = new QAccel( this );
    a->connectItem( a->insertItem( ALT + Key_B ), edit, SLOT( setFocus() ) );
    wspace = new Workspace( vbox, this );
    wspace->setBufferEdit( edit );
    wspace->setCurrentProject( currentProject );
    addToolBar( dw, Qt::DockRight );
    dw->setWidget( vbox );

    dw->setCaption( tr( "Project Overview" ) );
    QWhatsThis::add( wspace, tr("<b>The Project Overview Window</b>"
				"<p>The Project Overview Window displays all the current "
				"project, including forms and source files.</p>"
				"<p>Use the search field to rapidly switch between files.</p>"));
    dw->setFixedExtentHeight( 100 );
    dw->show();
}

void MainWindow::setupActionEditor()
{
    QDockWindow *dw = new QDockWindow( QDockWindow::OutsideDock, this, 0 );
    addDockWindow( dw, Qt::DockTornOff );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    actionEditor = new ActionEditor( dw );
    dw->setWidget( actionEditor );
    actionEditor->show();
    dw->setCaption( tr( "Action Editor" ) );
    QWhatsThis::add( actionEditor, tr("<b>The Action Editor</b>"
				      "<p>The Action Editor is used to add actions and action groups to "
				      "a form, and to connect actions to slots. Actions and action "
				      "groups can be dragged into menus and into toolbars, and may "
				      "feature keyboard shortcuts and tooltips. If actions have pixmaps "
				      "these are displayed on toolbar buttons and besides their names in "
				      "menus.</p>" ) );
    dw->hide();
    setAppropriate( dw, FALSE );
}

void MainWindow::setupToolbox()
{
    QDockWindow *dw = new QDockWindow( QDockWindow::InDock, this );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    addToolBar( dw, Qt::DockLeft );
    toolBox = new QToolBox( dw );
    dw->setWidget( toolBox );
    dw->setFixedExtentWidth( 160 );
    dw->setCaption( tr( "Toolbox" ) );
    dw->show();
    setDockEnabled( dw, Qt::DockTop, FALSE );
    setDockEnabled( dw, Qt::DockBottom, FALSE );
    commonWidgetsToolBar = new QToolBar( "Common Widgets", 0, toolBox, FALSE, "Common Widgets" );
    commonWidgetsToolBar->setFrameStyle( QFrame::NoFrame );
    commonWidgetsToolBar->setOrientation( Qt::Vertical );
    commonWidgetsToolBar->setBackgroundMode(PaletteBase);
    toolBox->addItem( commonWidgetsToolBar, "Common Widgets" );
}

void MainWindow::setupRMBMenus()
{
    rmbWidgets = new QPopupMenu( this );
    actionEditCut->addTo( rmbWidgets );
    actionEditCopy->addTo( rmbWidgets );
    actionEditPaste->addTo( rmbWidgets );
    actionEditDelete->addTo( rmbWidgets );
#if 0
    rmbWidgets->insertSeparator();
    actionEditLower->addTo( rmbWidgets );
    actionEditRaise->addTo( rmbWidgets );
#endif
    rmbWidgets->insertSeparator();
    actionEditAdjustSize->addTo( rmbWidgets );
    actionEditHLayout->addTo( rmbWidgets );
    actionEditVLayout->addTo( rmbWidgets );
    actionEditGridLayout->addTo( rmbWidgets );
    actionEditSplitHorizontal->addTo( rmbWidgets );
    actionEditSplitVertical->addTo( rmbWidgets );
    actionEditBreakLayout->addTo( rmbWidgets );
    rmbWidgets->insertSeparator();
    if ( !singleProjectMode() )
	actionEditConnections->addTo( rmbWidgets );
    actionEditSource->addTo( rmbWidgets );

    rmbFormWindow = new QPopupMenu( this );
    actionEditPaste->addTo( rmbFormWindow );
    actionEditSelectAll->addTo( rmbFormWindow );
    actionEditAccels->addTo( rmbFormWindow );
    rmbFormWindow->insertSeparator();
    actionEditAdjustSize->addTo( rmbFormWindow );
    actionEditHLayout->addTo( rmbFormWindow );
    actionEditVLayout->addTo( rmbFormWindow );
    actionEditGridLayout->addTo( rmbFormWindow );
    actionEditBreakLayout->addTo( rmbFormWindow );
    rmbFormWindow->insertSeparator();
    if ( !singleProjectMode() ) {
	actionEditFunctions->addTo( rmbFormWindow );
	actionEditConnections->addTo( rmbFormWindow );
    }
    actionEditSource->addTo( rmbFormWindow );
    rmbFormWindow->insertSeparator();
    actionEditFormSettings->addTo( rmbFormWindow );
}

void MainWindow::toolSelected( QAction* action )
{
    actionCurrentTool = action;
    emit currentToolChanged();
    if ( formWindow() )
	formWindow()->commandHistory()->emitUndoRedo();
}

int MainWindow::currentTool() const
{
    if ( !actionCurrentTool )
	return POINTER_TOOL;
    return QString::fromLatin1(actionCurrentTool->name()).toInt();
}

void MainWindow::runProjectPrecondition()
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	e->save();
	e->saveBreakPoints();
    }
    fileSaveProject();

    if ( currentTool() == ORDER_TOOL )
	resetTool();
    oWindow->clearErrorMessages();
    oWindow->clearDebug();
    oWindow->showDebugTab();
    previewing = TRUE;
}

void MainWindow::runProjectPostcondition( QObjectList *l )
{
    inDebugMode = TRUE;
    debuggingForms = *l;
    enableAll( FALSE );
    for ( SourceEditor *e2 = sourceEditors.first(); e2; e2 = sourceEditors.next() ) {
	if ( e2->project() == currentProject )
	    e2->editorInterface()->setMode( EditorInterface::Debugging );
    }
}

QWidget* MainWindow::previewFormInternal( QStyle* style, QPalette* palet )
{
    qwf_execute_code = FALSE;
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() )
	e->save();
    if ( currentTool() == ORDER_TOOL )
	resetTool();

    FormWindow *fw = formWindow();
    if ( !fw )
	return 0;

    QStringList databases;
    QPtrDictIterator<QWidget> wit( *fw->widgets() );
    while ( wit.current() ) {
	QStringList lst = MetaDataBase::fakeProperty( wit.current(), "database" ).toStringList();
	if ( !lst.isEmpty() )
	    databases << lst [ 0 ];
	++wit;
    }

    if ( fw->project() ) {
	QStringList::ConstIterator it;
	for ( it = databases.begin(); it != databases.end(); ++it )
	    fw->project()->openDatabase( *it, FALSE );
    }
    QApplication::setOverrideCursor( WaitCursor );

    QCString s;
    QBuffer buffer( s );
    buffer.open( IO_WriteOnly );
    Resource resource( this );
    resource.setWidget( fw );
    QValueList<Resource::Image> images;
    resource.save( &buffer );

    buffer.close();
    buffer.open( IO_ReadOnly );

    QWidget *w = QWidgetFactory::create( &buffer );
    if ( w ) {
	previewedForm = w;
	if ( palet ) {
	    if ( style )
		style->polish( *palet );
	    w->setPalette( *palet );
	}

	if ( style )
	    w->setStyle( style );

	QObjectList *l = w->queryList( "QWidget" );
	for ( QObject *o = l->first(); o; o = l->next() ) {
	    if ( style )
		( (QWidget*)o )->setStyle( style );
	}
	delete l;

	w->move( fw->mapToGlobal( QPoint(0,0) ) );
	((MainWindow*)w )->setWFlags( WDestructiveClose );
	previewing = TRUE;
	w->show();
	previewing = FALSE;
	QApplication::restoreOverrideCursor();
	return w;
    }
    QApplication::restoreOverrideCursor();
    return 0;
}

void MainWindow::previewForm()
{
    QWidget* w = previewFormInternal();
    if ( w )
	w->show();
}

void MainWindow::previewForm( const QString & style )
{
    QStyle* st = QStyleFactory::create( style );
    QWidget* w = 0;
    if ( style == "Motif" ) {
	QPalette p( QColor( 192, 192, 192 ) );
	w = previewFormInternal( st, &p );
    } else if ( style == "Windows" ) {
	QPalette p( QColor( 212, 208, 200 ) );
	w = previewFormInternal( st, &p );
    } else if ( style == "Platinum" ) {
	QPalette p( QColor( 220, 220, 220 ) );
	w = previewFormInternal( st, &p );
    } else if ( style == "CDE" ) {
	QPalette p( QColor( 75, 123, 130 ) );
	p.setColor( QPalette::Active, QColorGroup::Base, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Inactive, QColorGroup::Base, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Disabled, QColorGroup::Base, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Active, QColorGroup::Highlight, Qt::white );
	p.setColor( QPalette::Active, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Inactive, QColorGroup::Highlight, Qt::white );
	p.setColor( QPalette::Inactive, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Disabled, QColorGroup::Highlight, Qt::white );
	p.setColor( QPalette::Disabled, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Active, QColorGroup::Foreground, Qt::white );
	p.setColor( QPalette::Active, QColorGroup::Text, Qt::white );
	p.setColor( QPalette::Active, QColorGroup::ButtonText, Qt::white );
	p.setColor( QPalette::Inactive, QColorGroup::Foreground, Qt::white );
	p.setColor( QPalette::Inactive, QColorGroup::Text, Qt::white );
	p.setColor( QPalette::Inactive, QColorGroup::ButtonText, Qt::white );
	p.setColor( QPalette::Disabled, QColorGroup::Foreground, Qt::lightGray );
	p.setColor( QPalette::Disabled, QColorGroup::Text, Qt::lightGray );
	p.setColor( QPalette::Disabled, QColorGroup::ButtonText, Qt::lightGray );

	w = previewFormInternal( st, &p );
    } else if ( style == "SGI" ) {
	QPalette p( QColor( 220, 220, 220 ) );
	w = previewFormInternal( st, &p );
    } else if ( style == "MotifPlus" ) {
	QColor gtkfg(0x00, 0x00, 0x00);
	QColor gtkdf(0x75, 0x75, 0x75);
	QColor gtksf(0xff, 0xff, 0xff);
	QColor gtkbs(0xff, 0xff, 0xff);
	QColor gtkbg(0xd6, 0xd6, 0xd6);
	QColor gtksl(0x00, 0x00, 0x9c);
	QColorGroup active(gtkfg,            // foreground
			   gtkbg,            // button
			   gtkbg.light(),    // light
			   gtkbg.dark(142),  // dark
			   gtkbg.dark(110),  // mid
			   gtkfg,            // text
			   gtkfg,            // bright text
			   gtkbs,            // base
			   gtkbg),           // background
	    disabled(gtkdf,            // foreground
		     gtkbg,            // button
		     gtkbg.light(), // light
		     gtkbg.dark(156),  // dark
		     gtkbg.dark(110),  // mid
		     gtkdf,            // text
		     gtkdf,            // bright text
		     gtkbs,            // base
		     gtkbg);           // background

	QPalette pal(active, disabled, active);

	pal.setColor(QPalette::Active, QColorGroup::Highlight,
		     gtksl);
	pal.setColor(QPalette::Active, QColorGroup::HighlightedText,
		     gtksf);
	pal.setColor(QPalette::Inactive, QColorGroup::Highlight,
		     gtksl);
	pal.setColor(QPalette::Inactive, QColorGroup::HighlightedText,
		     gtksf);
	pal.setColor(QPalette::Disabled, QColorGroup::Highlight,
		     gtksl);
	pal.setColor(QPalette::Disabled, QColorGroup::HighlightedText,
		     gtkdf);
	w = previewFormInternal( st, &pal );
    } else {
	w = previewFormInternal( st );
    }

    if ( !w )
	return;
    w->insertChild( st );
    w->show();
}

void MainWindow::helpContents()
{
    QWidget *focusWidget = qApp->focusWidget();
    bool showClassDocu = TRUE;
    while ( focusWidget ) {
	if ( focusWidget->isA( "PropertyList" ) ) {
	    showClassDocu = FALSE;
	    break;
	}
	focusWidget = focusWidget->parentWidget();
    }

    QString source = "designer-manual.html";
    if ( propertyDocumentation.isEmpty() ) {
	QString indexFile = documentationPath() + "/propertyindex";
	QFile f( indexFile );
	if ( f.open( IO_ReadOnly ) ) {
	    QTextStream ts( &f );
	    while ( !ts.eof() ) {
		QString s = ts.readLine();
		int from = s.find( "\"" );
		if ( from == -1 )
		    continue;
		int to = s.findRev( "\"" );
		if ( to == -1 )
		    continue;
		propertyDocumentation[ s.mid( from + 1, to - from - 1 ) ] = s.mid( to + 2 ) + "-prop";
	    }
	    f.close();
	}
    }

    if ( propertyEditor->widget() && !showClassDocu ) {
	if ( !propertyEditor->currentProperty().isEmpty() ) {
	    QMetaObject* mo = propertyEditor->metaObjectOfCurrentProperty();
	    QString s;
	    QString cp = propertyEditor->currentProperty();
	    if ( cp == "layoutMargin" ) {
		source = propertyDocumentation[ "QLayout/margin" ];
	    } else if ( cp == "layoutSpacing" ) {
		source = propertyDocumentation[ "QLayout/spacing" ];
	    } else if ( cp == "toolTip" ) {
		source = "qtooltip.html#details";
	    } else if ( mo && qstrcmp( mo->className(), "Spacer" ) == 0 ) {
		if ( cp != "name" )
		    source = "qsizepolicy.html#SizeType";
		else
		    source = propertyDocumentation[ "QObject/name" ];
	    } else {
		while ( mo && !propertyDocumentation.contains( ( s = QString( mo->className() ) + "/" + cp ) ) )
		    mo = mo->superClass();
		if ( mo )
		    source = "p:" + propertyDocumentation[s];
	    }
	}

	QString classname =  WidgetFactory::classNameOf( propertyEditor->widget() );
	if ( source.isEmpty() || source == "designer-manual.html" ) {
	    if ( classname.lower() == "spacer" )
		source = "qspaceritem.html#details";
	    else if ( classname == "QLayoutWidget" )
		source = "layout.html";
	    else
		source = QString( WidgetFactory::classNameOf( propertyEditor->widget() ) ).lower() + ".html#details";
	}
    } else if ( propertyEditor->widget() ) {
	source = QString( WidgetFactory::classNameOf( propertyEditor->widget() ) ).lower() + ".html#details";
    }

    if ( !source.isEmpty() )
	assistant->showPage( documentationPath() + source );
}

void MainWindow::helpManual()
{
    assistant->showPage( documentationPath() + "/designer-manual.html" );
}

void MainWindow::helpAbout()
{
    AboutDialog dlg( this, 0, TRUE );
    if ( singleProjectMode() ) {
	dlg.aboutPixmap->setText( "" );
	dlg.aboutVersion->setText( "" );
	dlg.aboutCopyright->setText( "" );
	LanguageInterface *iface = MetaDataBase::languageInterface( eProject->language() );
	dlg.aboutLicense->setText( iface->aboutText() );
    }
    dlg.aboutVersion->setText(QString("Version ") + QString(QT_VERSION_STR));
    dlg.resize( dlg.width(), dlg.layout()->heightForWidth(dlg.width()) );
    dlg.exec();
}

void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt( this, "Qt Designer" );
}

#if defined(_WS_WIN_)
#include <qt_windows.h>
#include <qprocess.h>
#endif

void MainWindow::helpRegister()
{
#if defined(_WS_WIN_)
    HKEY key;
    HKEY subkey;
    long res;
    DWORD type;
    DWORD size = 255;
    QString command;
    QString sub( "htmlfile\\shell" );
#if defined(UNICODE)
    if ( QApplication::winVersion() & Qt::WV_NT_based ) {
	unsigned char data[256];
	res = RegOpenKeyExW( HKEY_CLASSES_ROOT, NULL, 0, KEY_READ, &key );
	res = RegOpenKeyExW( key, (TCHAR*)sub.ucs2(), 0, KEY_READ, &subkey );
	res = RegQueryValueExW( subkey, NULL, NULL, &type, data, &size );
	command = qt_winQString( data ) + "\\command";
	size = 255;
	res = RegOpenKeyExW( subkey, (TCHAR*)command.ucs2(), 0, KEY_READ, &subkey );
	res = RegQueryValueExW( subkey, NULL, NULL, &type, data, &size );
	command = qt_winQString( data );
    } else
#endif
    {
	unsigned char data[256];
	res = RegOpenKeyExA( HKEY_CLASSES_ROOT, NULL, 0, KEY_READ, &key );
	res = RegOpenKeyExA( key, sub.local8Bit(), 0, KEY_READ, &subkey );
	res = RegQueryValueExA( subkey, NULL, NULL, &type, data, &size );
	command = QString::fromLocal8Bit( (const char*) data ) + "\\command";
	size = 255;
	res = RegOpenKeyExA( subkey, command.local8Bit(), 0, KEY_READ, &subkey );
	res = RegQueryValueExA( subkey, NULL, NULL, &type, data, &size );
	command = QString::fromLocal8Bit( (const char*) data );
    }

    res = RegCloseKey( subkey );
    res = RegCloseKey( key );

    QProcess process( command + " www.trolltech.com/products/download/eval/evaluation.html" );
    if ( !process.start() )
	QMessageBox::information( this, "Register Qt", "Launching your web browser failed.\n"
	"To register Qt, point your browser to www.trolltech.com/products/download/eval/evaluation.html" );
#endif
}

void MainWindow::showProperties( QObject *o )
{
    if ( !o->isWidgetType() ) {
	propertyEditor->setWidget( o, lastActiveFormWindow );
	if ( lastActiveFormWindow )
	    hierarchyView->setFormWindow( lastActiveFormWindow, lastActiveFormWindow->mainContainer() );
	else
	    hierarchyView->setFormWindow( 0, 0 );
	return;
    }
    QWidget *w = (QWidget*)o;
    setupHierarchyView();
    FormWindow *fw = (FormWindow*)isAFormWindowChild( w );
    if ( fw ) {
	if ( fw->numSelectedWidgets() > 1 ) {
	    QWidgetList wl = fw->selectedWidgets();
	    if ( wl.first() != w ) {
		wl.removeRef( w );
		wl.insert( 0, w );
	    }
	    propertyEditor->setWidget( new PropertyObject( wl ), fw );
	} else {
	    propertyEditor->setWidget( w, fw );
	}
	hierarchyView->setFormWindow( fw, w );
    } else {
	propertyEditor->setWidget( 0, 0 );
	hierarchyView->setFormWindow( 0, 0 );
    }

    if ( currentTool() == POINTER_TOOL && fw &&
	 ( !qworkspace->activeWindow() || !::qt_cast<SourceEditor*>(qworkspace->activeWindow()) ) )
	fw->setFocus();
}

void MainWindow::resetTool()
{
    actionPointerTool->setOn( TRUE );
}

void MainWindow::updateProperties( QObject * )
{
    if ( propertyEditor )
	propertyEditor->refetchData();
}

bool MainWindow::eventFilter( QObject *o, QEvent *e )
{
    if ( ::qt_cast<MenuBarEditor*>(o) || ::qt_cast<PopupMenuEditor*>(o) ||
	 ( o &&
	   ( ::qt_cast<MenuBarEditor*>(o->parent()) ||
	     ::qt_cast<PopupMenuEditor*>(o->parent()) ) ) ) {

	if ( e->type() == QEvent::Accel && ::qt_cast<PopupMenuEditor*>(o) ) {
	    return TRUE; // consume accel events
 	} else if ( e->type() == QEvent::MouseButtonPress && ::qt_cast<MenuBarEditor*>(o) ) {
	    QPoint pos = ((QMouseEvent*)e)->pos();
	    MenuBarEditor *m = ::qt_cast<MenuBarEditor*>(o);
	    showProperties( o );
	    if ( m->findItem( pos ) >= m->count() )
		m->setFocus();
	} else if ( e->type() == QEvent::MouseButtonPress && ::qt_cast<PopupMenuEditor*>(o) ) {
	    PopupMenuEditor *m = ::qt_cast<PopupMenuEditor*>(o);
	    PopupMenuEditorItem *i = m->at( ((QMouseEvent*)e)->pos() );
	    if ( m->find( i->action() ) != -1 && !i->isSeparator() )
		showProperties( i->action() );
	}
	return QMainWindow::eventFilter( o, e );
    }

    if ( !o || !e || !o->isWidgetType() )
	return QMainWindow::eventFilter( o, e );

    QWidget *w = 0;
    bool passiveInteractor;
    switch ( e->type() ) {
    case QEvent::AccelOverride:
	if ( ( (QKeyEvent*)e )->key() == Key_F1 &&
	     ( ( (QKeyEvent*)e )->state() & ShiftButton ) != ShiftButton ) {
	    w = (QWidget*)o;
	    while ( w ) {
		if ( ::qt_cast<PropertyList*>(w) )
		    break;
		w = w->parentWidget( TRUE );
	    }
	    if ( w ) {
		propertyEditor->propertyList()->showCurrentWhatsThis();
		( (QKeyEvent*)e )->accept();
		return TRUE;
	    }
	}
	break;
    case QEvent::Accel:
	if ( ( ( (QKeyEvent*)e )->key() == Key_A ||
	       ( (QKeyEvent*)e )->key() == Key_E ) &&
	     ( (QKeyEvent*)e )->state() & ControlButton ) {
	    if ( qWorkspace()->activeWindow() &&
		 ::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) ) {
		( (QKeyEvent*)e )->ignore();
		return TRUE;
	    }
	}
	break;
    case QEvent::ContextMenu:
    case QEvent::MouseButtonPress:
	if ( o && currentTool() == POINTER_TOOL &&
	     ( ::qt_cast<MenuBarEditor*>(o) ||
	       ::qt_cast<PopupMenuEditor*>(o) ||
	       ::qt_cast<QDesignerToolBar*>(o) ||
	       ( ::qt_cast<QComboBox*>(o) ||
		 ::qt_cast<QToolButton*>(o) ||
		 ::qt_cast<QDesignerToolBarSeparator*>(o) ) &&
	       o->parent()
	       && ( ::qt_cast<QDesignerToolBar*>(o->parent())
		   || ::qt_cast<QDesignerWidgetStack*>(o->parent())) ) ) {
	    QWidget *w = (QWidget*)o;
	    if ( ::qt_cast<QToolButton*>(w) ||
		 ::qt_cast<QComboBox*>(w) ||
		 ::qt_cast<PopupMenuEditor*>(w) ||
		 ::qt_cast<QDesignerToolBarSeparator*>(w) )
		w = w->parentWidget();
	    QWidget *pw = w->parentWidget();
	    while ( pw ) {
		if ( ::qt_cast<FormWindow*>(pw) ) {
		    ( (FormWindow*)pw )->emitShowProperties( w );
		    if ( !::qt_cast<QDesignerToolBar*>(o) )
			return ( !::qt_cast<QToolButton*>(o) &&
				 !::qt_cast<MenuBarEditor*>(o) &&
				 !::qt_cast<QComboBox*>(o) &&
				 !::qt_cast<QDesignerToolBarSeparator*>(o) );
		}
		pw = pw->parentWidget();
	    }
	}
	if ( o && ( ::qt_cast<QDesignerToolBar*>(o) || o->inherits("QDockWindowHandle") )
	     && e->type() == QEvent::ContextMenu )
	    break;
	if ( isAToolBarChild( o ) && currentTool() != CONNECT_TOOL && currentTool() != BUDDY_TOOL )
	    break;
	if ( ::qt_cast<QSizeGrip*>(o) )
	    break;
	if ( !( w = isAFormWindowChild( o ) ) ||
	     ::qt_cast<SizeHandle*>(o) ||
	     ::qt_cast<OrderIndicator*>(o) )
	    break;
	if ( !w->hasFocus() )
	    w->setFocus();
	passiveInteractor = WidgetFactory::isPassiveInteractor( o );
	if ( !passiveInteractor || currentTool() != ORDER_TOOL ) {
	    if( e->type() == QEvent::ContextMenu ) {
		( (FormWindow*)w )->handleContextMenu( (QContextMenuEvent*)e,
						       ( (FormWindow*)w )->designerWidget( o ) );
		return TRUE;
	    } else {
		( (FormWindow*)w )->handleMousePress( (QMouseEvent*)e,
						      ( (FormWindow*)w )->designerWidget( o ) );
	    }
	}
	lastPressWidget = (QWidget*)o;
	if ( passiveInteractor )
	    QTimer::singleShot( 0, formWindow(), SLOT( visibilityChanged() ) );
	if ( currentTool() == CONNECT_TOOL || currentTool() == BUDDY_TOOL )
	    return TRUE;
	return !passiveInteractor;
    case QEvent::MouseButtonRelease:
	lastPressWidget = 0;
	if ( isAToolBarChild( o )  && currentTool() != CONNECT_TOOL && currentTool() != BUDDY_TOOL )
	    break;
	if ( ::qt_cast<QSizeGrip*>(o) )
	    break;
	if ( !( w = isAFormWindowChild( o ) ) ||
	     ::qt_cast<SizeHandle*>(o) ||
	     ::qt_cast<OrderIndicator*>(o) )
	    break;
	passiveInteractor = WidgetFactory::isPassiveInteractor( o );
	if ( !passiveInteractor )
	    ( (FormWindow*)w )->handleMouseRelease( (QMouseEvent*)e,
						    ( (FormWindow*)w )->designerWidget( o ) );
	if ( passiveInteractor ) {
	    QTimer::singleShot( 0, this, SLOT( selectionChanged() ) );
	    QTimer::singleShot( 0, formWindow(), SLOT( visibilityChanged() ) );
	}
	return !passiveInteractor;
    case QEvent::MouseMove:
	if ( isAToolBarChild( o ) && currentTool() != CONNECT_TOOL && currentTool() != BUDDY_TOOL )
	    break;
	w = isAFormWindowChild( o );
	if ( lastPressWidget != (QWidget*)o && w &&
	     !::qt_cast<SizeHandle*>(o) && !::qt_cast<OrderIndicator*>(o) &&
	     !::qt_cast<PopupMenuEditor*>(o) && !::qt_cast<QMenuBar*>(o) &&
	     !::qt_cast<QSizeGrip*>(o) )
	    return TRUE;
	if ( o && ::qt_cast<QSizeGrip*>(o) )
	    break;
	if ( lastPressWidget != (QWidget*)o ||
	     ( !w || ::qt_cast<SizeHandle*>(o) ) ||
	       ::qt_cast<OrderIndicator*>(o) )
	    break;
	passiveInteractor = WidgetFactory::isPassiveInteractor( o );
	if ( !passiveInteractor )
	    ( (FormWindow*)w )->handleMouseMove( (QMouseEvent*)e,
						 ( (FormWindow*)w )->designerWidget( o ) );
	return !passiveInteractor;
    case QEvent::KeyPress:
	if ( ( (QKeyEvent*)e )->key() == Key_Escape && currentTool() != POINTER_TOOL ) {
	    resetTool();
	    return FALSE;
	}
	if ( ( (QKeyEvent*)e )->key() == Key_Escape && incrementalSearch->hasFocus() ) {
	    if ( ::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) ) {
		qWorkspace()->activeWindow()->setFocus();
		return TRUE;
	    }
	}
	if ( !( w = isAFormWindowChild( o ) ) ||
	     ::qt_cast<SizeHandle*>(o) ||
	     ::qt_cast<OrderIndicator*>(o) )
	    break;
	( (FormWindow*)w )->handleKeyPress( (QKeyEvent*)e, ( (FormWindow*)w )->designerWidget( o ) );
	if ( ((QKeyEvent*)e)->isAccepted() )
	    return TRUE;
	break;
    case QEvent::MouseButtonDblClick:
	if ( !( w = isAFormWindowChild( o ) ) ||
	     ::qt_cast<SizeHandle*>(o) ||
	     ::qt_cast<OrderIndicator*>(o) ) {
	    if ( ::qt_cast<QToolButton*>(o) && ((QToolButton*)o)->isOn() &&
		 o->parent() && ::qt_cast<QToolBar*>(o->parent()) && formWindow() )
		formWindow()->setToolFixed();
	    break;
	}
	if ( currentTool() == ORDER_TOOL ) {
	    ( (FormWindow*)w )->handleMouseDblClick( (QMouseEvent*)e,
						     ( (FormWindow*)w )->designerWidget( o ) );
	    return TRUE;
	}
	if ( !WidgetFactory::isPassiveInteractor( o ) && ( (FormWindow*)w )->formFile() )
	    return openEditor( ( (FormWindow*)w )->designerWidget( o ), (FormWindow*)w );
	return TRUE;
    case QEvent::KeyRelease:
	if ( !( w = isAFormWindowChild( o ) ) ||
	     ::qt_cast<SizeHandle*>(o) ||
	     ::qt_cast<OrderIndicator*>(o) )
	    break;
	( (FormWindow*)w )->handleKeyRelease( (QKeyEvent*)e, ( (FormWindow*)w )->designerWidget( o ) );
	if ( ((QKeyEvent*)e)->isAccepted() )
	    return TRUE;
	break;
    case QEvent::Hide:
	if ( !( w = isAFormWindowChild( o ) ) ||
	     ::qt_cast<SizeHandle*>(o) ||
	     ::qt_cast<OrderIndicator*>(o) )
	    break;
	if ( ( (FormWindow*)w )->isWidgetSelected( (QWidget*)o ) )
	    ( (FormWindow*)w )->selectWidget( (QWidget*)o, FALSE );
	break;
    case QEvent::Enter:
    case QEvent::Leave:
	if ( !( w = isAFormWindowChild( o ) ) ||
	     ::qt_cast<SizeHandle*>(o) ||
	     ::qt_cast<OrderIndicator*>(o) ||
	     ::qt_cast<MenuBarEditor*>(o) )
	    break;
	return TRUE;
    case QEvent::Resize:
    case QEvent::Move:
	if ( !( w = isAFormWindowChild( o ) ) ||
	     ::qt_cast<SizeHandle*>(o) ||
	     ::qt_cast<OrderIndicator*>(o) )
	    break;
	if ( WidgetFactory::layoutType( (QWidget*)o->parent() ) != WidgetFactory::NoLayout ) {
	    ( (FormWindow*)w )->updateSelection( (QWidget*)o );
	    if ( e->type() != QEvent::Resize )
		( (FormWindow*)w )->updateChildSelections( (QWidget*)o );
	}
	break;
    case QEvent::Close:
	if ( o->isWidgetType() && (QWidget*)o == (QWidget*)previewedForm ) {
	    if ( lastActiveFormWindow && lastActiveFormWindow->project() ) {
		QStringList lst =
		    MetaDataBase::fakeProperty( lastActiveFormWindow, "database" ).toStringList();
		lastActiveFormWindow->project()->closeDatabase( lst[ 0 ] );
	    }
	}
	break;
    case QEvent::DragEnter:
	if ( o == qWorkspace() || o == workspace() || o == workspace()->viewport() ) {
	    workspace()->contentsDragEnterEvent( (QDragEnterEvent*)e );
	    return TRUE;
	}
	break;
    case QEvent::DragMove:
	if ( o == qWorkspace() || o == workspace() || o == workspace()->viewport() ) {
	    workspace()->contentsDragMoveEvent( (QDragMoveEvent*)e );
	    return TRUE;
	}
	break;
    case QEvent::Drop:
	if ( o == qWorkspace() || o == workspace() || o == workspace()->viewport() ) {
	    workspace()->contentsDropEvent( (QDropEvent*)e );
	    return TRUE;
	}
	break;
    case QEvent::Show:
	if ( o != this )
	    break;
	if ( ((QShowEvent*)e)->spontaneous() )
	    break;
	QApplication::sendPostedEvents( qworkspace, QEvent::ChildInserted );
	showEvent( (QShowEvent*)e );
	checkTempFiles();
	return TRUE;
    case QEvent::Wheel:
	if ( !( w = isAFormWindowChild( o ) ) ||
	     ::qt_cast<SizeHandle*>(o) ||
	     ::qt_cast<OrderIndicator*>(o) )
	    break;
	return TRUE;
    case QEvent::FocusIn:
	if ( !::qt_cast<FormWindow*>(o) && isAFormWindowChild( o ) )
	    return TRUE; //FIXME
	if ( hierarchyView->formDefinitionView()->isRenaming()
	     && ( o->inherits( "Editor" ) || ::qt_cast<FormWindow*>(o) ) )
		QApplication::sendPostedEvents();
	if ( o->inherits( "Editor" ) ) {
	    QWidget *w = (QWidget*)o;
	    while ( w ) {
		if ( ::qt_cast<SourceEditor*>(w) )
		    break;
		w = w->parentWidget( TRUE );
	    }
	    if ( ::qt_cast<SourceEditor*>(w) )
		( (SourceEditor*)w )->checkTimeStamp();
	} else if ( ::qt_cast<FormWindow*>(o) ) {
	    FormWindow *fw = (FormWindow*)o;
	    if ( fw->formFile() && fw->formFile()->editor() )
		fw->formFile()->editor()->checkTimeStamp();
	}
	break;
    case QEvent::FocusOut:
	if ( !::qt_cast<FormWindow*>(o) && isAFormWindowChild( o ) )
	    return TRUE;
	break;
    default:
	return QMainWindow::eventFilter( o, e );
    }

    return QMainWindow::eventFilter( o, e );
}

QWidget *MainWindow::isAFormWindowChild( QObject *o ) const
{
    if ( ::qt_cast<QWizard*>(o->parent()) && !::qt_cast<QPushButton*>(o) )
	return 0;
    while ( o ) {
	if ( ::qt_cast<FormWindow*>(o) )
	    return (QWidget*)o;
	o = o->parent();
    }
    return 0;
}

QWidget *MainWindow::isAToolBarChild( QObject *o ) const
{
    while ( o ) {
	if ( ::qt_cast<QDesignerToolBar*>(o) )
	    return (QWidget*)o;
	if ( ::qt_cast<FormWindow*>(o) )
	    return 0;
	o = o->parent();
    }
    return 0;
}

FormWindow *MainWindow::formWindow()
{
    if ( qworkspace->activeWindow() ) {
	FormWindow *fw = 0;
	if ( ::qt_cast<FormWindow*>(qworkspace->activeWindow()) )
	    fw = (FormWindow*)qworkspace->activeWindow();
	else if ( lastActiveFormWindow &&
		    qworkspace->windowList().find( lastActiveFormWindow ) != -1)
	    fw = lastActiveFormWindow;
	return fw;
    }
    return 0;
}

void MainWindow::emitProjectSignals()
{
    emit hasNonDummyProject( !currentProject->isDummy() );
    emit hasActiveWindowOrProject( !!qworkspace->activeWindow() || !currentProject->isDummy() );
}

void MainWindow::insertFormWindow( FormWindow *fw )
{
    if ( fw )
	QWhatsThis::add( fw, tr( "<b>The Form Window</b>"
			       "<p>Use the various tools to add widgets or to change the layout "
			       "and behavior of the components in the form. Select one or multiple "
			       "widgets to move them or lay them out. If a single widget is chosen it can "
			       "be resized using the resize handles.</p>"
			       "<p>Changes in the <b>Property Editor</b> are visible at design time, "
			       "and you can preview the form in different styles.</p>"
			       "<p>You can change the grid resolution, or turn the grid off in the "
			       "<b>Preferences</b> dialog in the <b>Edit</b> menu."
			       "<p>You can have several forms open, and all open forms are listed "
			       "in the <b>Form List</b>.") );

    connect( fw, SIGNAL( showProperties( QObject * ) ),
	     this, SLOT( showProperties( QObject * ) ) );
    connect( fw, SIGNAL( updateProperties( QObject * ) ),
	     this, SLOT( updateProperties( QObject * ) ) );
    connect( this, SIGNAL( currentToolChanged() ),
	     fw, SLOT( currentToolChanged() ) );
    connect( fw, SIGNAL( selectionChanged() ),
	     this, SLOT( selectionChanged() ) );
    connect( fw, SIGNAL( undoRedoChanged( bool, bool, const QString &, const QString & ) ),
	     this, SLOT( updateUndoRedo( bool, bool, const QString &, const QString & ) ) );

    if ( !mblockNewForms ) {
    } else {
	fw->setProject( currentProject );
    }
    fw->show();
    fw->currentToolChanged();
    if ( fw->caption().isEmpty() && qstrlen( fw->name() )  )
	fw->setCaption( fw->name() );
    fw->mainContainer()->setCaption( fw->caption() );
    WidgetFactory::saveDefaultProperties( fw->mainContainer(),
					  WidgetDatabase::
					  idFromClassName( WidgetFactory::classNameOf( fw->mainContainer() ) ) );
    activeWindowChanged( fw );
    emit formWindowsChanged();
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->project() == fw->project() )
	    e->resetContext();
    }
}

void MainWindow::createNewProject( const QString &lang )
{
    Project *pro = new Project( "", "", projectSettingsPluginManager );
    pro->setLanguage( lang );


    if ( !openProjectSettings( pro ) ) {
	delete pro;
	return;
    }

    if ( !pro->isValid() ) {
	QMessageBox::information( this, tr("New Project"), tr( "Cannot create an invalid project." ) );
	delete pro;
	return;
    }

    QAction *a = new QAction( pro->makeRelative( pro->fileName() ),
			      pro->makeRelative( pro->fileName() ), 0, actionGroupProjects, 0, TRUE );
    projects.insert( a, pro );
    addRecentlyOpened( pro->makeAbsolute( pro->fileName() ), recentlyProjects );
    projectSelected( a );
}


bool MainWindow::unregisterClient( FormWindow *w )
{
    propertyEditor->closed( w );
    objectHierarchy()->closed( w );
    if ( w == lastActiveFormWindow )
	lastActiveFormWindow = 0;

    QPtrList<SourceEditor> waitingForDelete;
    waitingForDelete.setAutoDelete( TRUE );
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->object() == w )
	    waitingForDelete.append( e );
    }

    if ( actionEditor->form() == w ) {
	actionEditor->setFormWindow( 0 );
	actionEditor->parentWidget()->hide();
    }

    return TRUE;
}

void MainWindow::activeWindowChanged( QWidget *w )
{
    QWidget *old = formWindow();
    if ( ::qt_cast<FormWindow*>(w) ) {
	FormWindow *fw = (FormWindow*)w;
	FormWindow *ofw = lastActiveFormWindow;
	lastActiveFormWindow = fw;
	lastActiveFormWindow->updateUndoInfo();
	emit hasActiveForm( TRUE );
	if ( formWindow() ) {
	    formWindow()->emitShowProperties();
	    emit formModified( formWindow()->commandHistory()->isModified() );
	    if ( currentTool() != POINTER_TOOL )
		formWindow()->clearSelection();
	}
	workspace()->activeFormChanged( fw );
	setAppropriate( (QDockWindow*)actionEditor->parentWidget(),
			::qt_cast<QMainWindow*>(lastActiveFormWindow->mainContainer()) );
	if ( appropriate( (QDockWindow*)actionEditor->parentWidget() ) ) {
	    if ( actionEditor->wantToBeShown() )
		actionEditor->parentWidget()->show();
	} else {
	    QWidget *mc = 0;
	    if ( ofw && (mc = ofw->mainContainer()) && ::qt_cast<QMainWindow*>(mc) )
		actionEditor->setWantToBeShown( !actionEditor->parentWidget()->isHidden() );
	    actionEditor->parentWidget()->hide();
	}

	actionEditor->setFormWindow( lastActiveFormWindow );
	if ( wspace && fw->project() && fw->project() != currentProject ) {
	    for ( QMap<QAction*, Project *>::ConstIterator it = projects.begin(); it != projects.end(); ++it ) {
		if ( *it == fw->project() ) {
		    projectSelected( it.key() );
		    break;
		}
	    }
	}
	emit formWindowChanged();

    } else if ( w == propertyEditor ) {
	propertyEditor->resetFocus();
    } else if ( !lastActiveFormWindow ) {
	emit formWindowChanged();
	emit hasActiveForm( FALSE );
	actionEditUndo->setEnabled( FALSE );
	actionEditRedo->setEnabled( FALSE );
    }

    if ( !w ) {
	emit formWindowChanged();
	emit hasActiveForm( FALSE );
	propertyEditor->clear();
	hierarchyView->clear();
	updateUndoRedo( FALSE, FALSE, QString::null, QString::null );
    }

    selectionChanged();

    if ( ::qt_cast<SourceEditor*>(w) ) {
	SourceEditor *se = (SourceEditor*)w;
	QGuardedPtr<FormWindow> fw = se->formWindow();
	if ( se->formWindow() &&
	     lastActiveFormWindow != fw ) {
	    activeWindowChanged( se->formWindow() );
	}
	actionSearchFind->setEnabled( TRUE );
	actionSearchIncremetal->setEnabled( TRUE );
	actionSearchReplace->setEnabled( TRUE );
	actionSearchGotoLine->setEnabled( TRUE );
	incrementalSearch->setEnabled( TRUE );

	actionEditUndo->setEnabled( FALSE );
	actionEditRedo->setEnabled( FALSE );
	actionEditCut->setEnabled( TRUE );
	actionEditCopy->setEnabled( TRUE );
	actionEditPaste->setEnabled( TRUE );
	actionEditSelectAll->setEnabled( TRUE );
	actionEditUndo->setMenuText( tr( "&Undo" ) );
	actionEditUndo->setToolTip( textNoAccel( actionEditUndo->menuText()) );
	actionEditRedo->setMenuText( tr( "&Redo" ) );
	actionEditRedo->setToolTip( textNoAccel( actionEditRedo->menuText()) );
	if ( hierarchyView->sourceEditor() != w )
	    hierarchyView->showClasses( se );
	actionEditor->setFormWindow( 0 );
	if ( wspace && se->project() && se->project() != currentProject ) {
	    for ( QMap<QAction*, Project *>::ConstIterator it = projects.begin(); it != projects.end(); ++it ) {
		if ( *it == se->project() ) {
		    projectSelected( it.key() );
		    break;
		}
	    }
	}
	workspace()->activeEditorChanged( se );
    } else {
	actionSearchFind->setEnabled( FALSE );
	actionSearchIncremetal->setEnabled( FALSE );
	actionSearchReplace->setEnabled( FALSE );
	actionSearchGotoLine->setEnabled( FALSE );
	incrementalSearch->setEnabled( FALSE );
    }

    if ( currentTool() == ORDER_TOOL && w != old )
	emit currentToolChanged();

    emit hasActiveWindow( !!qworkspace->activeWindow() );
}

void MainWindow::updateUndoRedo( bool undoAvailable, bool redoAvailable,
				 const QString &undoCmd, const QString &redoCmd )
{
    if ( qWorkspace()->activeWindow() &&
	 ::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) )
	 return; // do not set a formWindow related command
    actionEditUndo->setEnabled( undoAvailable );
    actionEditRedo->setEnabled( redoAvailable );
    if ( !undoCmd.isEmpty() )
	actionEditUndo->setMenuText( tr( "&Undo: %1" ).arg( undoCmd ) );
    else
	actionEditUndo->setMenuText( tr( "&Undo: Not Available" ) );
    if ( !redoCmd.isEmpty() )
	actionEditRedo->setMenuText( tr( "&Redo: %1" ).arg( redoCmd ) );
    else
	actionEditRedo->setMenuText( tr( "&Redo: Not Available" ) );

    actionEditUndo->setToolTip( textNoAccel( actionEditUndo->menuText()) );
    actionEditRedo->setToolTip( textNoAccel( actionEditRedo->menuText()) );

    if ( currentTool() == ORDER_TOOL ) {
	actionEditUndo->setEnabled( FALSE );
	actionEditRedo->setEnabled( FALSE );
    }
}

void MainWindow::updateEditorUndoRedo()
{
    if ( !qWorkspace()->activeWindow() ||
	 !::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) )
	return;
    SourceEditor *editor = (SourceEditor*)qWorkspace()->activeWindow();
    actionEditUndo->setEnabled( editor->editIsUndoAvailable() );
    actionEditRedo->setEnabled( editor->editIsRedoAvailable() );
}

QWorkspace *MainWindow::qWorkspace() const
{
    return qworkspace;
}

void MainWindow::popupFormWindowMenu( const QPoint & gp, FormWindow *fw )
{
    QValueList<uint> ids;
    QMap<QString, int> commands;

    setupRMBSpecialCommands( ids, commands, fw );
    setupRMBProperties( ids, commands, fw );

    qApp->processEvents();
    int r = rmbFormWindow->exec( gp );

    handleRMBProperties( r, commands, fw );
    handleRMBSpecialCommands( r, commands, fw );

    for ( QValueList<uint>::ConstIterator i = ids.begin(); i != ids.end(); ++i )
	rmbFormWindow->removeItem( *i );
}

void MainWindow::popupWidgetMenu( const QPoint &gp, FormWindow * /*fw*/, QWidget * w)
{
    QValueList<uint> ids;
    QMap<QString, int> commands;

    setupRMBSpecialCommands( ids, commands, w );
    setupRMBProperties( ids, commands, w );

    qApp->processEvents();
    int r = rmbWidgets->exec( gp );

    handleRMBProperties( r, commands, w );
    handleRMBSpecialCommands( r, commands, w );

    for ( QValueList<uint>::ConstIterator i = ids.begin(); i != ids.end(); ++i )
	rmbWidgets->removeItem( *i );
}

void MainWindow::setupRMBProperties( QValueList<uint> &ids, QMap<QString, int> &props, QWidget *w )
{
    const QMetaProperty* text = w->metaObject()->property( w->metaObject()->findProperty( "text", TRUE ), TRUE );
    if ( text && qstrcmp( text->type(), "QString") != 0 )
	text = 0;
    const QMetaProperty* title = w->metaObject()->property( w->metaObject()->findProperty( "title", TRUE ), TRUE );
    if ( title && qstrcmp( title->type(), "QString") != 0 )
	title = 0;
    const QMetaProperty* pagetitle =
	w->metaObject()->property( w->metaObject()->findProperty( "pageTitle", TRUE ), TRUE );
    if ( pagetitle && qstrcmp( pagetitle->type(), "QString") != 0 )
	pagetitle = 0;
    const QMetaProperty* pixmap =
	w->metaObject()->property( w->metaObject()->findProperty( "pixmap", TRUE ), TRUE );
    if ( pixmap && qstrcmp( pixmap->type(), "QPixmap") != 0 )
	pixmap = 0;

    if ( text && text->designable(w) ||
	 title && title->designable(w) ||
	 pagetitle && pagetitle->designable(w) ||
	 pixmap && pixmap->designable(w) ) {
	int id = 0;
	if ( ids.isEmpty() )
	    ids << rmbWidgets->insertSeparator(0);
	if ( pixmap && pixmap->designable(w) ) {
	    ids << ( id = rmbWidgets->insertItem( tr("Choose Pixmap..."), -1, 0) );
	    props.insert( "pixmap", id );
	}
	if ( text && text->designable(w) && !::qt_cast<QTextEdit*>(w) ) {
	    ids << ( id = rmbWidgets->insertItem( tr("Edit Text..."), -1, 0) );
	    props.insert( "text", id );
	}
	if ( title && title->designable(w) ) {
	    ids << ( id = rmbWidgets->insertItem( tr("Edit Title..."), -1, 0) );
	    props.insert( "title", id );
	}
	if ( pagetitle && pagetitle->designable(w) ) {
	    ids << ( id = rmbWidgets->insertItem( tr("Edit Page Title..."), -1, 0) );
	    props.insert( "pagetitle", id );
	}
    }
}

#ifdef QT_CONTAINER_CUSTOM_WIDGETS
static QWidgetContainerInterfacePrivate *containerWidgetInterface( QWidget *w )
{
    WidgetInterface *iface = 0;
    widgetManager()->queryInterface( WidgetFactory::classNameOf( w ), &iface );
    if ( !iface )
	return 0;
    QWidgetContainerInterfacePrivate *iface2 = 0;
    iface->queryInterface( IID_QWidgetContainer, (QUnknownInterface**)&iface2 );
    if ( !iface2 )
	return 0;
    iface->release();
    return iface2;
}
#endif

void MainWindow::setupRMBSpecialCommands( QValueList<uint> &ids,
					  QMap<QString, int> &commands, QWidget *w )
{
    int id;

    if ( ::qt_cast<QTabWidget*>(w) ) {
	if ( ids.isEmpty() )
	    ids << rmbWidgets->insertSeparator( 0 );
	if ( ( (QDesignerTabWidget*)w )->count() > 1) {
	    ids << ( id = rmbWidgets->insertItem( tr("Delete Page"), -1, 0 ) );
	    commands.insert( "remove", id );
	}
	ids << ( id = rmbWidgets->insertItem( tr("Add Page"), -1, 0 ) );
	commands.insert( "add", id );
    } else if ( ::qt_cast<QWidgetStack*>(w) ) {
	if ( ids.isEmpty() )
	    ids << rmbWidgets->insertSeparator( 0 );
	if ( ( (QDesignerWidgetStack*)w )->count() > 1) {
	    ids << ( id = rmbWidgets->insertItem( tr("Previous Page"), -1, 0 ) );
	    commands.insert( "prevpage", id );
	    ids << ( id = rmbWidgets->insertItem( tr("Next Page"), -1, 0 ) );
	    ids << rmbWidgets->insertSeparator( 0 );
	    commands.insert( "nextpage", id );
	    ids << ( id = rmbWidgets->insertItem( tr("Delete Page"), -1, 0 ) );
	    commands.insert( "remove", id );
	}
	ids << ( id = rmbWidgets->insertItem( tr("Add Page"), -1, 0 ) );
	commands.insert( "add", id );
    } else if ( ::qt_cast<QToolBox*>(w) ) {
	if ( ids.isEmpty() )
	    ids << rmbWidgets->insertSeparator( 0 );
	if ( ( (QToolBox*)w )->count() > 1 ) {
	    ids << ( id = rmbWidgets->insertItem( tr("Delete Page"), -1, 0 ) );
	    commands.insert( "remove", id );
	}
	ids << ( id = rmbWidgets->insertItem( tr("Add Page"), -1, 0 ) );
	commands.insert( "add", id );
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
    } else if ( WidgetDatabase::
		isCustomPluginWidget( WidgetDatabase::
				      idFromClassName( WidgetFactory::classNameOf( w ) ) ) ) {
	QWidgetContainerInterfacePrivate *iface = containerWidgetInterface( w );
	if ( iface && iface->supportsPages( WidgetFactory::classNameOf( w ) ) ) {
	    if ( ids.isEmpty() )
		ids << rmbWidgets->insertSeparator( 0 );

	    if ( iface->count( WidgetFactory::classNameOf( w ), w ) > 1 ) {
		ids << ( id = rmbWidgets->insertItem( tr( "Delete Page" ), -1, 0 ) );
		commands.insert( "remove", id );
	    }
	    ids << ( id = rmbWidgets->insertItem( tr("Add Page"), -1, 0 ) );
	    commands.insert( "add", id );
	    if ( iface->currentIndex( WidgetFactory::classNameOf( w ), w ) != -1 ) {
		ids << ( id = rmbWidgets->insertItem( tr("Rename Current Page..."), -1, 0 ) );
		commands.insert( "rename", id );
	    }
	}
	if ( iface )
	    iface->release();
#endif // QT_CONTAINER_CUSTOM_WIDGETS
    }

    if ( WidgetFactory::hasSpecialEditor( WidgetDatabase::
			  idFromClassName( WidgetFactory::classNameOf( w ) ), w ) ) {
	if ( ids.isEmpty() )
	    ids << rmbWidgets->insertSeparator( 0 );
	ids << ( id = rmbWidgets->insertItem( tr("Edit..."), -1, 0 ) );
	commands.insert( "edit", id );
    }
}

void MainWindow::setupRMBSpecialCommands( QValueList<uint> &ids,
					  QMap<QString, int> &commands, FormWindow *fw )
{
    int id;

    if ( ::qt_cast<QWizard*>(fw->mainContainer()) ) {
	if ( ids.isEmpty() )
	    ids << rmbFormWindow->insertSeparator( 0 );

	if ( ( (QWizard*)fw->mainContainer() )->pageCount() > 1) {
	    ids << ( id = rmbFormWindow->insertItem( tr("Delete Page"), -1, 0 ) );
	    commands.insert( "remove", id );
	}

	ids << ( id = rmbFormWindow->insertItem( tr("Add Page"), -1, 0 ) );
	commands.insert( "add", id );

	ids << ( id = rmbFormWindow->insertItem( tr("Edit Page Title..."), -1, 0 ) );
	commands.insert( "rename", id );

	ids << ( id = rmbFormWindow->insertItem( tr("Edit Pages..."), -1, 0 ) );
	commands.insert( "edit", id );

    } else if ( ::qt_cast<QMainWindow*>(fw->mainContainer()) ) {
	if ( ids.isEmpty() )
	    ids << rmbFormWindow->insertSeparator( 0 );
	ids << ( id = rmbFormWindow->insertItem( tr( "Add Menu Item" ), -1, 0 ) );
	commands.insert( "add_menu_item", id );
	ids << ( id = rmbFormWindow->insertItem( tr( "Add Toolbar" ), -1, 0 ) );
	commands.insert( "add_toolbar", id );
    }
}

void MainWindow::handleRMBProperties( int id, QMap<QString, int> &props, QWidget *w )
{
    if ( id == props[ "text" ] ) {
	bool ok = FALSE;
	bool oldDoWrap = FALSE;
	if ( ::qt_cast<QLabel*>(w) ) {
	    int align = w->property( "alignment" ).toInt();
	    if ( align & WordBreak )
		oldDoWrap = TRUE;
	}
	bool doWrap = oldDoWrap;

	QString text;
	if ( ::qt_cast<QTextView*>(w) || ::qt_cast<QLabel*>(w) || ::qt_cast<QButton*>(w) ) {
	    text = MultiLineEditor::getText( this, w->property( "text" ).toString(), !::qt_cast<QButton*>(w), &doWrap );
	    ok = !text.isNull();
	} else {
	    text = QInputDialog::getText( tr("Text"), tr( "New text" ),
				  QLineEdit::Normal, w->property("text").toString(), &ok, this );
	}
	if ( ok ) {
	    if ( oldDoWrap != doWrap ) {
		QString pn( tr( "Set 'wordwrap' of '%1'" ).arg( w->name() ) );
		SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
								  "wordwrap", QVariant( oldDoWrap, 0 ),
								  QVariant( doWrap, 0 ), QString::null, QString::null );
		cmd->execute();
		formWindow()->commandHistory()->addCommand( cmd );
		MetaDataBase::setPropertyChanged( w, "wordwrap", TRUE );
	    }

	    QString pn( tr( "Set the 'text' of '%1'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "text", w->property( "text" ),
							      text, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "text", TRUE );
	}
    } else if ( id == props[ "title" ] ) {
	bool ok = FALSE;
	QString title = QInputDialog::getText( tr("Title"), tr( "New title" ),
			       QLineEdit::Normal, w->property("title").toString(), &ok, this );
	if ( ok ) {
	    QString pn( tr( "Set the 'title' of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "title", w->property( "title" ),
							      title, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "title", TRUE );
	}
    } else if ( id == props[ "pagetitle" ] ) {
	bool ok = FALSE;
	QString text = QInputDialog::getText( tr("Page Title"), tr( "New page title" ),
			      QLineEdit::Normal, w->property("pageTitle").toString(), &ok, this );
	if ( ok ) {
	    QString pn( tr( "Set the 'pageTitle' of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "pageTitle",
							      w->property( "pageTitle" ),
							      text, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "pageTitle", TRUE );
	}
    } else if ( id == props[ "pixmap" ] ) {
	QPixmap oldPix = w->property( "pixmap" ).toPixmap();
	QPixmap pix = qChoosePixmap( this, formWindow(), oldPix );
	if ( !pix.isNull() ) {
	    QString pn( tr( "Set the 'pixmap' of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "pixmap", w->property( "pixmap" ),
							      pix, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "pixmap", TRUE );
	}
    }
}

void MainWindow::handleRMBSpecialCommands( int id, QMap<QString, int> &commands, QWidget *w )
{
    if ( ::qt_cast<QTabWidget*>(w) ) {
	QTabWidget *tw = (QTabWidget*)w;
	if ( id == commands[ "add" ] ) {
	    AddTabPageCommand *cmd =
		new AddTabPageCommand( tr( "Add Page to %1" ).arg( tw->name() ), formWindow(),
				       tw, "Tab" );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	} else if ( id == commands[ "remove" ] ) {
	    if ( tw->currentPage() ) {
		QDesignerTabWidget *dtw = (QDesignerTabWidget*)tw;
		DeleteTabPageCommand *cmd =
		    new DeleteTabPageCommand( tr( "Delete Page %1 of %2" ).
					      arg( dtw->pageTitle() ).arg( tw->name() ),
					      formWindow(), tw, tw->currentPage() );
		formWindow()->commandHistory()->addCommand( cmd );
		cmd->execute();
	    }
	}
    } else if ( ::qt_cast<QToolBox*>(w) ) {
	QToolBox *tb = (QToolBox*)w;
	if ( id == commands[ "add" ] ) {
	    AddToolBoxPageCommand *cmd =
		new AddToolBoxPageCommand( tr( "Add Page to %1" ).arg( tb->name() ),
					   formWindow(),
					   tb, "Page" );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	} else if ( id == commands[ "remove" ] ) {
	    if ( tb->currentItem() ) {
		DeleteToolBoxPageCommand *cmd =
		    new DeleteToolBoxPageCommand( tr( "Delete Page %1 of %2" ).
					  arg( tb->itemLabel( tb->currentIndex() ) ).
					  arg( tb->name() ),
					  formWindow(), tb, tb->currentItem() );
		formWindow()->commandHistory()->addCommand( cmd );
		cmd->execute();
	    }
	}
    } else if ( ::qt_cast<QWidgetStack*>(w) ) {
	QDesignerWidgetStack *ws = (QDesignerWidgetStack*)w;
	if ( id == commands[ "add" ] ) {
	    AddWidgetStackPageCommand *cmd =
		new AddWidgetStackPageCommand( tr( "Add Page to %1" ).arg( ws->name() ),
					       formWindow(), ws );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	} else if ( id == commands[ "remove" ] ) {
	    if ( ws->visibleWidget() ) {
		DeleteWidgetStackPageCommand *cmd =
		    new DeleteWidgetStackPageCommand( tr( "Delete Page %1 of %2" ).
						      arg( ws->currentPage() ).arg( ws->name() ),
						      formWindow(), ws, ws->visibleWidget() );
		formWindow()->commandHistory()->addCommand( cmd );
		cmd->execute();
	    }
	} else if ( id == commands[ "nextpage" ] ) {
	    int currentPage = w->property( "currentPage" ).toInt();
	    QString pn( tr( "Raise next page of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd =
		new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
					"currentPage", currentPage,
					currentPage + 1, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "currentPage", TRUE );
	} else if ( id == commands[ "prevpage" ] ) {
	    int currentPage = w->property( "currentPage" ).toInt();
	    QString pn( tr( "Raise previous page of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd =
		new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
					"currentPage", currentPage,
					currentPage -1, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "currentPage", TRUE );
	}
#ifdef QT_CONTAINER_CUSTOM_WIDGETS
    } else if ( WidgetDatabase::
		isCustomPluginWidget( WidgetDatabase::
				      idFromClassName( WidgetFactory::classNameOf( w ) ) ) ) {
	if ( id == commands[ "add" ] ) {
	    AddContainerPageCommand *cmd =
		new AddContainerPageCommand( tr( "Add Page to %1" ).arg( w->name() ),
					     formWindow(), w, "Page" );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	} else if ( id == commands[ "remove" ] ) {
	    QWidgetContainerInterfacePrivate *iface = containerWidgetInterface( w );
	    if ( iface ) {
		QString wClassName = WidgetFactory::classNameOf( w );
		int index = iface->currentIndex( wClassName, w );
		DeleteContainerPageCommand *cmd =
		    new DeleteContainerPageCommand( tr( "Delete Page %1 of %2" ).
						    arg( iface->pageLabel( wClassName,
									   w, index ) ).
						    arg( w->name() ),
						    formWindow(), w, index );
		formWindow()->commandHistory()->addCommand( cmd );
		cmd->execute();
		iface->release();
	    }
	} else if ( id == commands[ "rename" ] ) {
	    QWidgetContainerInterfacePrivate *iface = containerWidgetInterface( w );
	    if ( iface ) {
		bool ok = FALSE;
		QString wClassName = WidgetFactory::classNameOf( w );
		int index = iface->currentIndex( wClassName, w );
		QString text = QInputDialog::getText( tr("Page Title"), tr( "New page title" ),
						      QLineEdit::Normal,
						      iface->pageLabel( wClassName, w,
								       index ),
						      &ok, this );
		if ( ok ) {
		    RenameContainerPageCommand *cmd =
			new RenameContainerPageCommand( tr( "Rename Page %1 to %2" ).
							arg( iface->
							     pageLabel( wClassName, w,
									index ) ).
							arg( text ), formWindow(),
							w, index, text );
		    formWindow()->commandHistory()->addCommand( cmd );
		    cmd->execute();
		}
		iface->release();
	    }
	}
#endif // QT_CONTAINER_CUSTOM_WIDGETS
    }

    if ( WidgetFactory::hasSpecialEditor( WidgetDatabase::
				  idFromClassName( WidgetFactory::classNameOf( w ) ), w ) ) {
	if ( id == commands[ "edit" ] )
	    WidgetFactory::editWidget( WidgetDatabase::
		       idFromClassName( WidgetFactory::classNameOf( w ) ), this, w, formWindow() );
    }
}

void MainWindow::handleRMBSpecialCommands( int id, QMap<QString, int> &commands, FormWindow *fw )
{
    if ( ::qt_cast<QWizard*>(fw->mainContainer()) ) {
	QWizard *wiz = (QWizard*)fw->mainContainer();
	if ( id == commands[ "add" ] ) {
	    AddWizardPageCommand *cmd =
		new AddWizardPageCommand( tr( "Add Page to %1" ).arg( wiz->name() ),
					  formWindow(), wiz, "WizardPage" );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	} else if ( id == commands[ "remove" ] ) {
	    if ( wiz->currentPage() ) {
		QDesignerWizard *dw = (QDesignerWizard*)wiz;
		DeleteWizardPageCommand *cmd =
		    new DeleteWizardPageCommand( tr( "Delete Page %1 of %2" ).
						 arg( dw->pageTitle() ).arg( wiz->name() ),
						 formWindow(), wiz,
						 wiz->indexOf( wiz->currentPage() ) );
		formWindow()->commandHistory()->addCommand( cmd );
		cmd->execute();
	    }
	} else if ( id == commands[ "edit" ] ) {
	    WizardEditor *e = new WizardEditor( this, wiz, fw );
	    e->exec();
	    delete e;
	} else if ( id == commands[ "rename" ] ) {

	    bool ok = FALSE;
	    QDesignerWizard *dw = (QDesignerWizard*)wiz;
	    QString text = QInputDialog::getText( tr("Page Title"), tr( "New page title" ),
					  QLineEdit::Normal, dw->pageTitle(), &ok, this );
	    if ( ok ) {
		QString pn( tr( "Rename page %1 of %2" ).arg( dw->pageTitle() ).arg( wiz->name() ) );
		RenameWizardPageCommand *cmd =
		    new RenameWizardPageCommand( pn, formWindow()
						 , wiz, wiz->indexOf( wiz->currentPage() ), text );
		formWindow()->commandHistory()->addCommand( cmd );
		cmd->execute();
	    }
	}
    } else if ( ::qt_cast<QMainWindow*>(fw->mainContainer()) ) {
	QMainWindow *mw = (QMainWindow*)fw->mainContainer();
	if ( id == commands[ "add_toolbar" ] ) {
	    AddToolBarCommand *cmd =
		new AddToolBarCommand( tr( "Add Toolbar to '%1'" ).arg( formWindow()->name() ),
				       formWindow(), mw );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	} else if ( id == commands[ "add_menu_item" ] ) {
	    AddMenuCommand *cmd =
		new AddMenuCommand( tr( "Add Menu to '%1'" ).arg( formWindow()->name() ),
				    formWindow(), mw );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	}
    }
}

void MainWindow::clipboardChanged()
{
    QString text( qApp->clipboard()->text() );
    QString start( "<!DOCTYPE UI-SELECTION>" );
    actionEditPaste->setEnabled( text.left( start.length() ) == start );
}

void MainWindow::selectionChanged()
{
    layoutChilds = FALSE;
    layoutSelected = FALSE;
    breakLayout = FALSE;
    if ( !formWindow() ) {
	actionEditCut->setEnabled( FALSE );
	actionEditCopy->setEnabled( FALSE );
	actionEditDelete->setEnabled( FALSE );
	actionEditAdjustSize->setEnabled( FALSE );
	actionEditHLayout->setEnabled( FALSE );
	actionEditVLayout->setEnabled( FALSE );
	actionEditSplitHorizontal->setEnabled( FALSE );
	actionEditSplitVertical->setEnabled( FALSE );
	actionEditGridLayout->setEnabled( FALSE );
	actionEditBreakLayout->setEnabled( FALSE );
	actionEditLower->setEnabled( FALSE );
	actionEditRaise->setEnabled( FALSE );
	actionEditAdjustSize->setEnabled( FALSE );
	return;
    }

    int selectedWidgets = formWindow()->numSelectedWidgets();
    bool enable = selectedWidgets > 0;
    actionEditCut->setEnabled( enable );
    actionEditCopy->setEnabled( enable );
    actionEditDelete->setEnabled( enable );
    actionEditLower->setEnabled( enable );
    actionEditRaise->setEnabled( enable );

    actionEditAdjustSize->setEnabled( FALSE );
    actionEditSplitHorizontal->setEnabled( FALSE );
    actionEditSplitVertical->setEnabled( FALSE );

    enable = FALSE;
    QWidgetList widgets = formWindow()->selectedWidgets();
    if ( selectedWidgets > 1 ) {
	int unlaidout = 0;
	int laidout = 0;
	for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	    if ( !w->parentWidget() || WidgetFactory::layoutType( w->parentWidget() ) == WidgetFactory::NoLayout )
		unlaidout++;
	    else
		laidout++;
	}
	actionEditHLayout->setEnabled( unlaidout > 1 );
	actionEditVLayout->setEnabled( unlaidout > 1 );
	actionEditSplitHorizontal->setEnabled( unlaidout > 1 );
	actionEditSplitVertical->setEnabled( unlaidout > 1 );
	actionEditGridLayout->setEnabled( unlaidout > 1 );
	actionEditBreakLayout->setEnabled( laidout > 0 );
	actionEditAdjustSize->setEnabled( laidout > 0 );
	layoutSelected = unlaidout > 1;
	breakLayout = laidout > 0;
    } else if ( selectedWidgets == 1 ) {
	QWidget *w = widgets.first();
	bool isContainer = WidgetDatabase::isContainer( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ) ) ||
			   w == formWindow()->mainContainer();
	actionEditAdjustSize->setEnabled( !w->parentWidget() ||
					  WidgetFactory::layoutType( w->parentWidget() ) == WidgetFactory::NoLayout );

	if ( !isContainer ) {
	    actionEditHLayout->setEnabled( FALSE );
	    actionEditVLayout->setEnabled( FALSE );
	    actionEditGridLayout->setEnabled( FALSE );
	    if ( w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout ) {
		actionEditBreakLayout->setEnabled( !isAToolBarChild( w ) );
		breakLayout = TRUE;
	    } else {
		actionEditBreakLayout->setEnabled( FALSE );
	    }
	} else {
	    if ( WidgetFactory::layoutType( w ) == WidgetFactory::NoLayout ) {
		if ( !formWindow()->hasInsertedChildren( w ) ) {
		    actionEditHLayout->setEnabled( FALSE );
		    actionEditVLayout->setEnabled( FALSE );
		    actionEditGridLayout->setEnabled( FALSE );
		    actionEditBreakLayout->setEnabled( FALSE );
		} else {
		    actionEditHLayout->setEnabled( TRUE );
		    actionEditVLayout->setEnabled( TRUE );
		    actionEditGridLayout->setEnabled( TRUE );
		    actionEditBreakLayout->setEnabled( FALSE );
		    layoutChilds = TRUE;
		}
		if ( w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout ) {
		    actionEditBreakLayout->setEnabled( !isAToolBarChild( w ) );
		    breakLayout = TRUE;
		}
	    } else {
		actionEditHLayout->setEnabled( FALSE );
		actionEditVLayout->setEnabled( FALSE );
		actionEditGridLayout->setEnabled( FALSE );
		actionEditBreakLayout->setEnabled( !isAToolBarChild( w ) );
		breakLayout = TRUE;
	    }
	}
    } else if ( selectedWidgets == 0 ) {
	actionEditAdjustSize->setEnabled( TRUE );
	QWidget *w = formWindow()->mainContainer();
	if ( WidgetFactory::layoutType( w ) == WidgetFactory::NoLayout ) {
	    if ( !formWindow()->hasInsertedChildren( w ) ) {
		actionEditHLayout->setEnabled( FALSE );
		actionEditVLayout->setEnabled( FALSE );
		actionEditGridLayout->setEnabled( FALSE );
		actionEditBreakLayout->setEnabled( FALSE );
	    } else {
		actionEditHLayout->setEnabled( TRUE );
		actionEditVLayout->setEnabled( TRUE );
		actionEditGridLayout->setEnabled( TRUE );
		actionEditBreakLayout->setEnabled( FALSE );
		layoutChilds = TRUE;
	    }
	} else {
	    actionEditHLayout->setEnabled( FALSE );
	    actionEditVLayout->setEnabled( FALSE );
	    actionEditGridLayout->setEnabled( FALSE );
	    actionEditBreakLayout->setEnabled( TRUE );
	    breakLayout = TRUE;
	}
    } else {
	actionEditHLayout->setEnabled( FALSE );
	actionEditVLayout->setEnabled( FALSE );
	actionEditGridLayout->setEnabled( FALSE );
	actionEditBreakLayout->setEnabled( FALSE );
    }
}

static QString fixArgs( const QString &s2 )
{
    QString s = s2;
    return s.replace( ',', ';' );
}

void MainWindow::writeConfig()
{
    QSettings config;

    // No search path for unix, only needs application name
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QString keybase = DesignerApplication::settingsKey();

    if (savePluginPaths) {
	QStringList pluginPaths = QApplication::libraryPaths();
	config.writeEntry( keybase + "PluginPaths", pluginPaths );
    }
    config.writeEntry( keybase + "RestoreWorkspace", restoreConfig );
    config.writeEntry( keybase + "SplashScreen", splashScreen );
    config.writeEntry( keybase + "ShowStartDialog", shStartDialog );
    config.writeEntry( keybase + "FileFilter", fileFilter );
    config.writeEntry( keybase + "TemplatePath", templPath );
    config.writeEntry( keybase + "RecentlyOpenedFiles", recentlyFiles );
    config.writeEntry( keybase + "RecentlyOpenedProjects", recentlyProjects );
    config.writeEntry( keybase + "DatabaseAutoEdit", databaseAutoEdit );

    config.writeEntry( keybase + "AutoSave/Enabled", autoSaveEnabled );
    config.writeEntry( keybase + "AutoSave/Interval", autoSaveInterval );

    config.writeEntry( keybase + "Grid/Snap", snGrid );
    config.writeEntry( keybase + "Grid/Show", sGrid );
    config.writeEntry( keybase + "Grid/x", grid().x() );
    config.writeEntry( keybase + "Grid/y", grid().y() );
    config.writeEntry( keybase + "LastToolPage",
		       toolBox->itemLabel( toolBox->currentIndex() ) );

    config.writeEntry( keybase + "Background/UsePixmap", backPix );
    config.writeEntry( keybase + "Background/Color", (int)qworkspace->backgroundColor().rgb() );
    if ( qworkspace->backgroundPixmap() )
	qworkspace->backgroundPixmap()->save( QDir::home().absPath() + "/.designer/" + "background.xpm", "PNG" );

    config.writeEntry( keybase + "Geometries/MainwindowX", x() );
    config.writeEntry( keybase + "Geometries/MainwindowY", y() );
    config.writeEntry( keybase + "Geometries/MainwindowWidth", width() );
    config.writeEntry( keybase + "Geometries/MainwindowHeight", height() );
    config.writeEntry( keybase + "Geometries/MainwindowMaximized", isMaximized() );
    config.writeEntry( keybase + "Geometries/PropertyEditorX", propertyEditor->parentWidget()->x() );
    config.writeEntry( keybase + "Geometries/PropertyEditorY", propertyEditor->parentWidget()->y() );
    config.writeEntry( keybase + "Geometries/PropertyEditorWidth", propertyEditor->parentWidget()->width() );
    config.writeEntry( keybase + "Geometries/PropertyEditorHeight", propertyEditor->parentWidget()->height() );
    config.writeEntry( keybase + "Geometries/HierarchyViewX", hierarchyView->parentWidget()->x() );
    config.writeEntry( keybase + "Geometries/HierarchyViewY", hierarchyView->parentWidget()->y() );
    config.writeEntry( keybase + "Geometries/HierarchyViewWidth", hierarchyView->parentWidget()->width() );
    config.writeEntry( keybase + "Geometries/HierarchyViewHeight", hierarchyView->parentWidget()->height() );
    config.writeEntry( keybase + "Geometries/WorkspaceX", wspace->parentWidget()->x() );
    config.writeEntry( keybase + "Geometries/WorkspaceY", wspace->parentWidget()->y() );
    config.writeEntry( keybase + "Geometries/WorkspaceWidth", wspace->parentWidget()->width() );
    config.writeEntry( keybase + "Geometries/WorkspaceHeight", wspace->parentWidget()->height() );

    config.writeEntry( keybase + "View/TextLabels", usesTextLabel() );
    config.writeEntry( keybase + "View/BigIcons", usesBigPixmaps() );

    QString mwlKey = "MainwindowLayout";
    if ( singleProjectMode() )
	mwlKey += "S";
    QString mainWindowLayout;
    QTextStream ts( &mainWindowLayout, IO_WriteOnly );
    ts << *this;
    config.writeEntry( keybase + mwlKey, mainWindowLayout );


    QPtrList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();
    config.writeEntry( keybase + "CustomWidgets/num", (int)lst->count() );
    int j = 0;
    QDir::home().mkdir( ".designer" );
    for ( MetaDataBase::CustomWidget *w = lst->first(); w; w = lst->next() ) {
	QStringList l;
	l << w->className;
	l << w->includeFile;
	l << QString::number( (int)w->includePolicy );
	l << QString::number( w->sizeHint.width() );
	l << QString::number( w->sizeHint.height() );
	l << QString::number( w->lstSignals.count() );
	for ( QValueList<QCString>::ConstIterator it = w->lstSignals.begin(); it != w->lstSignals.end(); ++it )
	    l << QString( fixArgs( *it ) );
	l << QString::number( w->lstSlots.count() );
	for ( QValueList<MetaDataBase::Function>::ConstIterator it2 = w->lstSlots.begin(); it2 != w->lstSlots.end(); ++it2 ) {
	    l << fixArgs( (*it2).function );
	    l << (*it2).access;
	}
	l << QString::number( w->lstProperties.count() );
	for ( QValueList<MetaDataBase::Property>::ConstIterator it3 = w->lstProperties.begin(); it3 != w->lstProperties.end(); ++it3 ) {
	    l << (*it3).property;
	    l << (*it3).type;
	}
	l << QString::number( size_type_to_int( w->sizePolicy.horData() ) );
	l << QString::number( size_type_to_int( w->sizePolicy.verData() ) );
	l << QString::number( (int)w->isContainer );
	config.writeEntry( keybase + "CustomWidgets/Widget" + QString::number( j++ ), l, ',' );
	w->pixmap->save( QDir::home().absPath() + "/.designer/" + w->className, "XPM" );
    }

    QStringList l;
    for ( QAction *a = commonWidgetsPage.first(); a; a = commonWidgetsPage.next() )
	l << a->text();
    config.writeEntry( keybase + "ToolBox/CommonWidgets", l );
}

static QString fixArgs2( const QString &s2 )
{
    QString s = s2;
    return s.replace( ';', ',' );
}

void MainWindow::readConfig()
{
    QString keybase = DesignerApplication::settingsKey();
    QSettings config;
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    bool ok;
    bool readPreviousConfig = FALSE;
    QString backPixName( QDir::home().absPath() + "/.designer/" + "background.xpm" );
    restoreConfig = config.readBoolEntry( keybase + "RestoreWorkspace", TRUE, &ok );
    if ( !ok ) {
	keybase = DesignerApplication::oldSettingsKey();
	restoreConfig = config.readBoolEntry( keybase + "RestoreWorkspace", TRUE, &ok );
	if ( !ok ) {
	    if ( oWindow ) {
		oWindow->shuttingDown();
		( (QDockWindow*)oWindow->parent() )->hide();
	    }
	    QPixmap pix;
	    pix.load( backPixName );
	    if ( !pix.isNull() )
		qworkspace->setBackgroundPixmap( pix );
	    return;
	}
	readPreviousConfig = TRUE;
    }
    if ( !readPreviousConfig ) {
	fileFilter = config.readEntry( keybase + "FileFilter", fileFilter );
	templPath = config.readEntry( keybase + "TemplatePath", QString::null );
	databaseAutoEdit = config.readBoolEntry( keybase + "DatabaseAutoEdit", databaseAutoEdit );
	shStartDialog = config.readBoolEntry( keybase + "ShowStartDialog", shStartDialog );
	autoSaveEnabled = config.readBoolEntry( keybase + "AutoSave/Enabled", autoSaveEnabled );
	autoSaveInterval = config.readNumEntry( keybase + "AutoSave/Interval", autoSaveInterval );
    }

    if ( restoreConfig || readPreviousConfig ) {
	QString s = config.readEntry( keybase + "LastToolPage" );
	for ( int i = 0; i < toolBox->count(); ++i ) {
	    if ( toolBox->itemLabel(i) == s ) {
		toolBox->setCurrentIndex( i );
		break;
	    }
	}
	// We know that the oldSettingsKey() will return 3.1
	if ( keybase == DesignerApplication::oldSettingsKey() ) {
	    if (keybase.contains("3.1"))
		recentlyFiles = config.readListEntry( keybase + "RecentlyOpenedFiles", ',' );
	    else
		recentlyFiles = config.readListEntry(keybase + "RecentlyOpenedFiles");

	    if ( recentlyFiles.count() == 1 && recentlyFiles[0].isNull() )
		recentlyFiles.clear();
	    if (keybase.contains("3.1"))
		recentlyProjects = config.readListEntry( keybase + "RecentlyOpenedProjects", ',' );
	    else
		recentlyProjects = config.readListEntry( keybase + "RecentlyOpenedProjects");
	    if ( recentlyProjects.count() == 1 && recentlyProjects[0].isNull() )
		recentlyProjects.clear();
	} else {
	    recentlyFiles = config.readListEntry( keybase + "RecentlyOpenedFiles" );
	    recentlyProjects = config.readListEntry( keybase + "RecentlyOpenedProjects" );
	}

	backPix = config.readBoolEntry( keybase + "Background/UsePixmap", TRUE ) | readPreviousConfig;
	if ( backPix ) {
	    QPixmap pix;
	    pix.load( backPixName );
	    if ( !pix.isNull() )
		qworkspace->setBackgroundPixmap( pix );
	} else {
	    qworkspace->setBackgroundColor( QColor( (QRgb)config.readNumEntry( keybase + "Background/Color" ) ) );
	}

	if ( !readPreviousConfig ) {
	    splashScreen = config.readBoolEntry( keybase + "SplashScreen", TRUE );

	    sGrid = config.readBoolEntry( keybase + "Grid/Show", TRUE );
	    snGrid = config.readBoolEntry( keybase + "Grid/Snap", TRUE );
	    grd.setX( config.readNumEntry( keybase + "Grid/x", 10 ) );
	    grd.setY( config.readNumEntry( keybase + "Grid/y", 10 ) );

	    if ( !config.readBoolEntry( DesignerApplication::settingsKey() + "Geometries/MainwindowMaximized", FALSE ) ) {
		QRect r( pos(), size() );
		r.setX( config.readNumEntry( keybase + "Geometries/MainwindowX", r.x() ) );
		r.setY( config.readNumEntry( keybase + "Geometries/MainwindowY", r.y() ) );
		r.setWidth( config.readNumEntry( keybase + "Geometries/MainwindowWidth", r.width() ) );
		r.setHeight( config.readNumEntry( keybase + "Geometries/MainwindowHeight", r.height() ) );

		QRect desk = QApplication::desktop()->geometry();
		QRect inter = desk.intersect( r );
		resize( r.size() );
		if ( inter.width() * inter.height() > ( r.width() * r.height() / 20 ) ) {
		    move( r.topLeft() );
		}
	    }
	    setUsesTextLabel( config.readBoolEntry( keybase + "View/TextLabels", FALSE ) );
	    setUsesBigPixmaps( FALSE /*config.readBoolEntry( "BigIcons", FALSE )*/ ); // ### disabled for now
	}
    }
    int num = config.readNumEntry( keybase + "CustomWidgets/num" );
    for ( int j = 0; j < num; ++j ) {
	MetaDataBase::CustomWidget *w = new MetaDataBase::CustomWidget;
	QStringList l = config.readListEntry( keybase + "CustomWidgets/Widget" + QString::number( j ), ',' );
	w->className = l[ 0 ];
	w->includeFile = l[ 1 ];
	w->includePolicy = (MetaDataBase::CustomWidget::IncludePolicy)l[ 2 ].toInt();
	w->sizeHint.setWidth( l[ 3 ].toInt() );
	w->sizeHint.setHeight( l[ 4 ].toInt() );
	uint c = 5;
	if ( l.count() > c ) {
	    int numSignals = l[ c ].toInt();
	    c++;
	    for ( int i = 0; i < numSignals; ++i, c++ )
		w->lstSignals.append( fixArgs2( l[ c ] ).latin1() );
	}
	if ( l.count() > c ) {
	    int numSlots = l[ c ].toInt();
	    c++;
	    for ( int i = 0; i < numSlots; ++i ) {
		MetaDataBase::Function slot;
		slot.function = fixArgs2( l[ c ] );
		c++;
		slot.access = l[ c ];
		c++;
		w->lstSlots.append( slot );
	    }
	}
	if ( l.count() > c ) {
	    int numProperties = l[ c ].toInt();
	    c++;
	    for ( int i = 0; i < numProperties; ++i ) {
		MetaDataBase::Property prop;
		prop.property = l[ c ];
		c++;
		prop.type = l[ c ];
		c++;
		w->lstProperties.append( prop );
	    }
	} if ( l.count() > c ) {
	    QSizePolicy::SizeType h, v;
	     h = int_to_size_type( l[ c++ ].toInt() );
	     v = int_to_size_type( l[ c++ ].toInt() );
	     w->sizePolicy = QSizePolicy( h, v );
	}
	if ( l.count() > c ) {
	    w->isContainer = (bool)l[ c++ ].toInt();
	}
	w->pixmap = new QPixmap( BarIcon( QDir::home().absPath() + "/.designer/" + w->className , KDevDesignerPartFactory::instance()) );
	MetaDataBase::addCustomWidget( w );
    }
    if ( num > 0 )
	rebuildCustomWidgetGUI();

    if ( !restoreConfig )
	return;

#ifndef Q_WS_MAC
    /* I'm sorry to make this not happen on the Mac, but it seems to hang somewhere deep
       in QLayout, it gets into a very large loop - and seems it has to do with clever
       things the designer does ###Sam */
    QApplication::sendPostedEvents();
#endif

    if ( !readPreviousConfig ) {
	QString mwlKey = "MainwindowLayout";
	if ( singleProjectMode() )
	    mwlKey += "S";
	QString mainWindowLayout = config.readEntry( keybase + mwlKey );
	QTextStream ts( &mainWindowLayout, IO_ReadOnly );
	ts >> *this;
    }
    if ( readPreviousConfig && oWindow ) {
	oWindow->shuttingDown();
	( (QDockWindow*)oWindow->parent() )->hide();
    }

    rebuildCustomWidgetGUI();

    QStringList l = config.readListEntry( keybase + "ToolBox/CommonWidgets" );
    if ( !l.isEmpty() ) {
	QPtrList<QAction> lst;
	commonWidgetsPage.clear();
	for ( QStringList::ConstIterator it = l.begin(); it != l.end(); ++it ) {
	    for ( QAction *a = toolActions.first(); a; a = toolActions.next() ) {
		if ( *it == a->text() ) {
		    lst.append( a );
		    break;
		}
	    }
	}
	if ( lst != commonWidgetsPage ) {
	    commonWidgetsPage = lst;
	    rebuildCommonWidgetsToolBoxPage();
	}
    }
}

HierarchyView *MainWindow::objectHierarchy() const
{
    if ( !hierarchyView )
	( (MainWindow*)this )->setupHierarchyView();
    return hierarchyView;
}

QPopupMenu *MainWindow::setupNormalHierarchyMenu( QWidget *parent )
{
    QPopupMenu *menu = new QPopupMenu( parent );

    actionEditCut->addTo( menu );
    actionEditCopy->addTo( menu );
    actionEditPaste->addTo( menu );
    actionEditDelete->addTo( menu );

    return menu;
}

QPopupMenu *MainWindow::setupTabWidgetHierarchyMenu( QWidget *parent, const char *addSlot, const char *removeSlot )
{
    QPopupMenu *menu = new QPopupMenu( parent );

    menu->insertItem( tr( "Add Page" ), parent, addSlot );
    menu->insertItem( tr( "Delete Page" ), parent, removeSlot );
    menu->insertSeparator();
    actionEditCut->addTo( menu );
    actionEditCopy->addTo( menu );
    actionEditPaste->addTo( menu );
    actionEditDelete->addTo( menu );

    return menu;
}

void MainWindow::closeEvent( QCloseEvent *e )
{
    if ( singleProject ) {
	hide();
	e->ignore();
	return;
    }

    QWidgetList windows = qWorkspace()->windowList();
    QWidgetListIt wit( windows );
    while ( wit.current() ) {
	QWidget *w = wit.current();
	++wit;
	if ( ::qt_cast<FormWindow*>(w) ) {
	    if ( ( (FormWindow*)w )->formFile()->editor() )
		windows.removeRef( ( (FormWindow*)w )->formFile()->editor() );
	    if ( ( (FormWindow*)w )->formFile()->formWindow() )
		windows.removeRef( ( (FormWindow*)w )->formFile()->formWindow() );
	    if ( !( (FormWindow*)w )->formFile()->close() ) {
		e->ignore();
		return;
	    }
	} else if ( ::qt_cast<SourceEditor*>(w) ) {
	    if ( !( (SourceEditor*)w )->close() ) {
		e->ignore();
		return;
	    }
	}
	w->close();
    }

    QMapConstIterator<QAction*, Project*> it = projects.begin();
    while( it != projects.end() ) {
	Project *pro = it.data();
	++it;
	if ( pro->isModified() ) {
	    switch ( QMessageBox::warning( this, tr( "Save Project Settings" ),
					   tr( "Save changes to '%1'?" ).arg( pro->fileName() ),
					   tr( "&Yes" ), tr( "&No" ), tr( "&Cancel" ), 0, 2 ) ) {
	    case 0: // save
		pro->save();
		break;
	    case 1: // don't save
		break;
	    case 2: // cancel
		e->ignore();
		return;
	    default:
		break;
	    }
	}
    }

    writeConfig();
    hide();
    e->accept();

    if ( client ) {
	QDir home( QDir::homeDirPath() );
	home.remove( ".designerpid" );
    }
}

Workspace *MainWindow::workspace() const
{
    if ( !wspace )
	( (MainWindow*)this )->setupWorkspace();
    return wspace;
}

PropertyEditor *MainWindow::propertyeditor() const
{
    if ( !propertyEditor )
	( (MainWindow*)this )->setupPropertyEditor();
    return propertyEditor;
}

ActionEditor *MainWindow::actioneditor() const
{
    if ( !actionEditor )
	( (MainWindow*)this )->setupActionEditor();
    return actionEditor;
}

bool MainWindow::openEditor( QWidget *w, FormWindow *f )
{
    if ( f && !f->project()->isCpp() && !WidgetFactory::isPassiveInteractor( w ) ) {
	QString defSignal = WidgetFactory::defaultSignal( w );
	if ( defSignal.isEmpty() ) {
	    editSource();
	} else {
	    QString s = QString( w->name() ) + "_" + defSignal;
	    LanguageInterface *iface = MetaDataBase::languageInterface( f->project()->language() );
	    if ( iface ) {
		QStrList sigs = iface->signalNames( w );
		QString fullSignal;
		for ( int i = 0; i < (int)sigs.count(); ++i ) {
		    QString sig = sigs.at( i );
		    if ( sig.left( sig.find( '(' ) ) == defSignal ) {
			fullSignal = sig;
			break;
		    }
		}

		if ( !fullSignal.isEmpty() ) {
		    QString signl = fullSignal;
		    fullSignal = fullSignal.mid( fullSignal.find( '(' ) + 1 );
		    fullSignal.remove( (int)fullSignal.length() - 1, 1 );
		    fullSignal = iface->createArguments( fullSignal.simplifyWhiteSpace() );
		    s += "(" + fullSignal + ")";
		    if ( !MetaDataBase::hasFunction( f, s.latin1() ) )
			MetaDataBase::addFunction( f, s.latin1(), "", "public", "slot",
						   f->project()->language(), "void" );
		    s = s.left( s.find( '(' ) ).latin1();
		    if ( !MetaDataBase::hasConnection( f, w, defSignal.latin1(), f->mainContainer(), s.latin1() ) ) {
			MetaDataBase::Connection conn;
			conn.sender = w;
			conn.receiver = f->mainContainer();
			conn.signal = signl;
			conn.slot = s;
			AddConnectionCommand *cmd =
			    new AddConnectionCommand( tr( "Add connection" ), f, conn );
			f->commandHistory()->addCommand( cmd );
			cmd->execute();
			f->formFile()->setModified( TRUE );
		    }
		}
	    }
	    editFunction( s, TRUE );
	}
	return TRUE;
    }
    if ( WidgetFactory::hasSpecialEditor( WidgetDatabase::
					  idFromClassName( WidgetFactory::classNameOf( w ) ), w ) ) {
	statusMessage( tr( "Edit %1..." ).arg( w->className() ) );
	WidgetFactory::editWidget( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ),
				   this, w, formWindow() );
	statusBar()->clear();
	return TRUE;
    }

    const QMetaProperty* text = w->metaObject()->property( w->metaObject()->findProperty( "text", TRUE ), TRUE );
    const QMetaProperty* title = w->metaObject()->property( w->metaObject()->findProperty( "title", TRUE ), TRUE );
    if ( text && text->designable(w) ) {
	bool ok = FALSE;
	bool oldDoWrap = FALSE;
	if ( ::qt_cast<QLabel*>(w) ) {
	    int align = w->property( "alignment" ).toInt();
	    if ( align & WordBreak )
		oldDoWrap = TRUE;
	}
	bool doWrap = oldDoWrap;

	QString text;
	if ( ::qt_cast<QTextEdit*>(w) || ::qt_cast<QLabel*>(w) || ::qt_cast<QButton*>(w) ) {
	    text = MultiLineEditor::getText( this, w->property( "text" ).toString(),
					     !::qt_cast<QButton*>(w), &doWrap );
	    ok = !text.isNull();
	} else {
	    text = QInputDialog::getText( tr("Text"), tr( "New text" ),
				  QLineEdit::Normal, w->property("text").toString(), &ok, this );
	}
	if ( ok ) {
	    if ( oldDoWrap != doWrap ) {
		QString pn( tr( "Set 'wordwrap' of '%1'" ).arg( w->name() ) );
		SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
								  "wordwrap", QVariant( oldDoWrap, 0 ),
								  QVariant( doWrap, 0 ), QString::null, QString::null );
		cmd->execute();
		formWindow()->commandHistory()->addCommand( cmd );
		MetaDataBase::setPropertyChanged( w, "wordwrap", TRUE );
	    }

	    QString pn( tr( "Set the 'text' of '%1'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "text", w->property( "text" ),
							      text, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "text", TRUE );
	}
	return TRUE;
    }
    if ( title && title->designable(w) ) {
	bool ok = FALSE;
	QString text;
	text = QInputDialog::getText( tr("Title"), tr( "New title" ), QLineEdit::Normal, w->property("title").toString(), &ok, this );
	if ( ok ) {
	    QString pn( tr( "Set the 'title' of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "title", w->property( "title" ),
							      text, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "title", TRUE );
	}
	return TRUE;
    }

    if ( !WidgetFactory::isPassiveInteractor( w ) )
	editSource();

    return TRUE;
}

void MainWindow::rebuildCustomWidgetGUI()
{
    customWidgetToolBar->clear();
    customWidgetMenu->clear();
    customWidgetToolBar2->clear();
    int count = 0;

    QPtrListIterator<QAction> it( toolActions );
    QAction *action;
    while ( ( action = it.current() ) ) {
	++it;
	if ( ( (WidgetAction*)action )->group() == "Custom Widgets" )
	    delete action;
    }

    QPtrList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();

    actionToolsCustomWidget->addTo( customWidgetMenu );
    customWidgetMenu->insertSeparator();

    for ( MetaDataBase::CustomWidget *w = lst->first(); w; w = lst->next() ) {
	WidgetAction* a = new WidgetAction( "Custom Widgets", actionGroupTools, QString::number( w->id ).latin1() );
	a->setToggleAction( TRUE );
	a->setText( w->className );
	a->setIconSet( *w->pixmap );
	a->setStatusTip( tr( "Insert a " +w->className + " (custom widget)" ) );
	a->setWhatsThis( tr("<b>" + w->className + " (custom widget)</b>"
			    "<p>Click <b>Edit Custom Widgets...</b> in the <b>Tools|Custom</b> menu to "
			    "add and change custom widgets. You can add properties as well as "
			    "signals and slots to integrate them into Qt Designer, "
			    "and provide a pixmap which will be used to represent the widget on the form.</p>") );

	a->addTo( customWidgetToolBar );
	a->addTo( customWidgetToolBar2 );
	a->addTo( customWidgetMenu );
	count++;
    }
    QWidget *wid;
    customWidgetToolBar2->setStretchableWidget( ( wid = new QWidget( customWidgetToolBar2 ) ) );
    wid->setBackgroundMode( customWidgetToolBar2->backgroundMode() );

    if ( count == 0 )
	customWidgetToolBar->hide();
    else if ( customWidgetToolBar->isVisible() )
	customWidgetToolBar->show();
}

void MainWindow::rebuildCommonWidgetsToolBoxPage()
{
    toolBox->setUpdatesEnabled( FALSE );
    commonWidgetsToolBar->setUpdatesEnabled( FALSE );
    commonWidgetsToolBar->clear();
    for ( QAction *a = commonWidgetsPage.first(); a; a = commonWidgetsPage.next() )
	a->addTo( commonWidgetsToolBar );
    QWidget *w;
    commonWidgetsToolBar->setStretchableWidget( ( w = new QWidget( commonWidgetsToolBar ) ) );
    w->setBackgroundMode( commonWidgetsToolBar->backgroundMode() );
    toolBox->setUpdatesEnabled( TRUE );
    commonWidgetsToolBar->setUpdatesEnabled( TRUE );
}

bool MainWindow::isCustomWidgetUsed( MetaDataBase::CustomWidget *wid )
{
    QWidgetList windows = qWorkspace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( ::qt_cast<FormWindow*>(w) ) {
	    if ( ( (FormWindow*)w )->isCustomWidgetUsed( wid ) )
		return TRUE;
	}
    }
    return FALSE;
}

void MainWindow::setGrid( const QPoint &p )
{
    if ( p == grd )
	return;
    grd = p;
    QWidgetList windows = qWorkspace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( !::qt_cast<FormWindow*>(w) )
	    continue;
	( (FormWindow*)w )->mainContainer()->update();
    }
}

void MainWindow::setShowGrid( bool b )
{
    if ( b == sGrid )
	return;
    sGrid = b;
    QWidgetList windows = qWorkspace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( !::qt_cast<FormWindow*>(w) )
	    continue;
	( (FormWindow*)w )->mainContainer()->update();
    }
}

void MainWindow::setSnapGrid( bool b )
{
    if ( b == snGrid )
	return;
    snGrid = b;
}

QString MainWindow::documentationPath() const
{
    return QString( qInstallPathDocs() ) + "/html/";
}

void MainWindow::windowsMenuActivated( int id )
{
    QWidget* w = qworkspace->windowList().at( id );
    if ( w )
	w->setFocus();
}

void MainWindow::projectSelected( QAction *a )
{
    a->setOn( TRUE );
    if ( currentProject )
	currentProject->setActive( FALSE );
    Project *p = *projects.find( a );
    p->setActive( TRUE );
    if ( currentProject == p )
	return;
    currentProject = p;
    if ( wspace )
	wspace->setCurrentProject( currentProject );
}

void MainWindow::openProject( const QString &fn )
{
    for ( QMap<QAction*, Project*>::ConstIterator it = projects.begin(); it != projects.end(); ++it ) {
	if ( (*it)->fileName() == fn ) {
	    projectSelected( it.key() );
	    return;
	}
    }
    QApplication::setOverrideCursor( waitCursor );
    Project *pro = new Project( fn, "", projectSettingsPluginManager );
    pro->setModified( FALSE );
    QAction *a = new QAction( pro->projectName(), pro->projectName(), 0, actionGroupProjects, 0, TRUE );
    projects.insert( a, pro );
    projectSelected( a );
    QApplication::restoreOverrideCursor();
}

void MainWindow::checkTempFiles()
{
    QString s = QDir::homeDirPath() + "/.designer";
    QString baseName = s+ "/saved-form-";
    if ( !QFile::exists( baseName + "1.ui" ) )
	return;
    DesignerApplication::closeSplash();
    QDir d( s );
    d.setNameFilter( "*.ui" );
    QStringList lst = d.entryList();
    QApplication::restoreOverrideCursor();
    bool load = QMessageBox::information( this, tr( "Restoring the Last Session" ),
					  tr( "Qt Designer found some temporary saved files, which were\n"
					      "written when Qt Designer crashed last time. Do you want to\n"
					      "load these files?" ), tr( "&Yes" ), tr( "&No" ) ) == 0;
    QApplication::setOverrideCursor( waitCursor );
    for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
	if ( load )
	    openFormWindow( s + "/" + *it, FALSE );
	d.remove( *it );
    }
}

void MainWindow::showDialogHelp()
{
    QWidget *w = (QWidget*)sender();
    w = w->topLevelWidget();

    QString link = documentationPath() + "/designer-manual-13.html#";

    if ( ::qt_cast<NewFormBase*>(w) || ::qt_cast<StartDialogBase*>(w) ) // own doc for startdialog?
	link += "dialog-file-new";
    else if ( ::qt_cast<CreateTemplate*>(w) )
	link += "dialog-file-create-template";
    else if ( ::qt_cast<EditFunctionsBase*>(w) )
	link += "dialog-edit-functions";
//     else if ( ::qt_cast<ConnectionViewerBase*>(w) )
    else if ( w->inherits("ConnectionViewerBase") )
 	link += "dialog-view-connections";
    else if ( ::qt_cast<FormSettingsBase*>(w) )
	link += "dialog-edit-form-settings";
    else if ( ::qt_cast<Preferences*>(w) )
	link += "dialog-edit-preferences";
    else if ( ::qt_cast<PixmapCollectionEditor*>(w) )
	link += "dialog-image-collection";
//    else if ( ::qt_cast<DatabaseConnectionBase*>(w) )
    else if ( w->inherits( "DatabaseConnectionBase" ) )
	link += "dialog-edit-database-connections";
    else if ( ::qt_cast<ProjectSettingsBase*>(w) )
	link += "dialog-project-settings";
    else if ( ::qt_cast<FindDialog*>(w) )
	link += "dialog-find-text";
    else if ( ::qt_cast<ReplaceDialog*>(w) )
	link += "dialog-replace-text";
    else if ( ::qt_cast<GotoLineDialog*>(w) )
	link += "dialog-go-to-line";
//    else if ( ::qt_cast<ConnectionEditorBase*>(w) )
    else if ( w->inherits("ConnectionEditorBase") )
	link += "dialog-edit-connections";
    else if ( ::qt_cast<CustomWidgetEditorBase*>(w) )
	link += "dialog-edit-custom-widgets";
    else if ( ::qt_cast<PaletteEditorBase*>(w) )
	link += "dialog-edit-palette";
    else if ( ::qt_cast<ListBoxEditorBase*>(w) )
	link += "dialog-edit-listbox";
    else if ( ::qt_cast<ListViewEditorBase*>(w) )
	link += "dialog-edit-listview";
    else if ( ::qt_cast<IconViewEditorBase*>(w) )
	link += "dialog-edit-iconview";
    else if ( ::qt_cast<TableEditorBase*>(w) )
	link += "dialog-edit-table";
    else if ( ::qt_cast<MultiLineEditor*>(w) )
	link += "dialog-text";

    else {
	QMessageBox::information( this, tr( "Help" ),
				  tr( "There is no help available for this dialog at the moment." ) );
	return;
    }

    assistant->showPage( link );
}

void MainWindow::setupActionManager()
{
    actionPluginManager = new QPluginManager<ActionInterface>( IID_Action, QApplication::libraryPaths(), pluginDirectory() );

    QStringList lst = actionPluginManager->featureList();
    for ( QStringList::ConstIterator ait = lst.begin(); ait != lst.end(); ++ait ) {
	ActionInterface *iface = 0;
	actionPluginManager->queryInterface( *ait, &iface );
	if ( !iface )
	    continue;

	iface->connectTo( desInterface );
	QAction *a = iface->create( *ait, this );
	if ( !a )
	    continue;

	QString grp = iface->group( *ait );
	if ( grp.isEmpty() )
	    grp = "3rd party actions";
	QPopupMenu *menu = 0;
	QToolBar *tb = 0;

	if ( !( menu = (QPopupMenu*)child( grp.latin1(), "QPopupMenu" ) ) ) {
	    menu = new QPopupMenu( this, grp.latin1() );
	    menuBar()->insertItem( tr( grp ), menu );
	}
	if ( !( tb = (QToolBar*)child( grp.latin1(), "QToolBar" ) ) ) {
	    tb = new QToolBar( this, grp.latin1() );
	    tb->setCloseMode( QDockWindow::Undocked );
	    addToolBar( tb, grp );
	}

	if ( iface->location( *ait, ActionInterface::Menu ) )
	    a->addTo( menu );
	if ( iface->location( *ait, ActionInterface::Toolbar ) )
	    a->addTo( tb );

	iface->release();
    }
}

void MainWindow::editFunction( const QString &func, bool rereadSource )
{
    if ( !formWindow() )
	return;

    if ( formWindow()->formFile()->codeFileState() != FormFile::Ok )
	if ( !formWindow()->formFile()->setupUihFile(FALSE) )
	    return;

    QString lang = currentProject->language();
    if ( !MetaDataBase::hasEditor( lang ) ) {
	QMessageBox::information( this, tr( "Edit Source" ),
				  tr( "There is no plugin for editing " + lang + " code installed!\n"
				      "Note: Plugins are not available in static Qt configurations." ) );
	return;
    }

    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->language() == lang && e->formWindow() == formWindow() ) {
	    e->show();
	    e->setFunction( func );
	    return;
	}
    }

    createSourceEditor( formWindow(), formWindow()->project(), lang, func, rereadSource );
}

void MainWindow::setupRecentlyFilesMenu()
{
    recentlyFilesMenu->clear();
    int id = 0;
    for ( QStringList::ConstIterator it = recentlyFiles.begin(); it != recentlyFiles.end(); ++it ) {
	recentlyFilesMenu->insertItem( *it, id );
	id++;
    }
}

void MainWindow::setupRecentlyProjectsMenu()
{
    recentlyProjectsMenu->clear();
    int id = 0;
    for ( QStringList::ConstIterator it = recentlyProjects.begin(); it != recentlyProjects.end(); ++it ) {
	recentlyProjectsMenu->insertItem( *it, id );
	id++;
    }
}

QPtrList<DesignerProject> MainWindow::projectList() const
{
    QPtrList<DesignerProject> list;
    QMapConstIterator<QAction*, Project*> it = projects.begin();

    while( it != projects.end() ) {
	Project *p = it.data();
	++it;
	list.append( p->iFace() );
    }

    return list;
}

QStringList MainWindow::projectNames() const
{
    QStringList res;
    for ( QMap<QAction*, Project* >::ConstIterator it = projects.begin(); it != projects.end(); ++it )
	res << (*it)->projectName();
    return res;
}

QStringList MainWindow::projectFileNames() const
{
    QStringList res;
    for ( QMap<QAction*, Project* >::ConstIterator it = projects.begin(); it != projects.end(); ++it )
	res << (*it)->makeRelative( (*it)->fileName() );
    return res;
}

Project *MainWindow::findProject( const QString &projectName ) const
{
    for ( QMap<QAction*, Project* >::ConstIterator it = projects.begin(); it != projects.end(); ++it ) {
	if ( (*it)->projectName() == projectName )
	    return *it;
    }
    return 0;
}

void MainWindow::setCurrentProject( Project *pro )
{
    for ( QMap<QAction*, Project* >::ConstIterator it = projects.begin(); it != projects.end(); ++it ) {
	if ( *it == pro ) {
	    projectSelected( it.key() );
	    return;
	}
    }
}

void MainWindow::setCurrentProjectByFilename( const QString& proFilename )
{
    for ( QMap<QAction*, Project* >::ConstIterator it = projects.begin(); it != projects.end(); ++it ) {
	if ( (*it)->makeRelative( (*it)->fileName() ) == proFilename ) {
	    projectSelected( it.key() );
	    return;
	}
    }
}


void MainWindow::recentlyFilesMenuActivated( int id )
{
    if ( id != -1 ) {
	if ( !QFile::exists( *recentlyFiles.at( id ) ) ) {
	    QMessageBox::warning( this, tr( "Open File" ),
				  tr( "Could not open '%1'. File does not exist." ).
				  arg( *recentlyFiles.at( id ) ) );
	    recentlyFiles.remove( recentlyFiles.at( id ) );
	    return;
	}
	fileOpen( "", "", *recentlyFiles.at( id ) );
	QString fn( *recentlyFiles.at( id ) );
	addRecentlyOpened( fn, recentlyFiles );
    }
}

void MainWindow::recentlyProjectsMenuActivated( int id )
{
    if ( id != -1 ) {
	if ( !QFile::exists( *recentlyProjects.at( id ) ) ) {
	    QMessageBox::warning( this, tr( "Open Project" ),
				  tr( "Could not open '%1'. File does not exist." ).
				  arg( *recentlyProjects.at( id ) ) );
	    recentlyProjects.remove( recentlyProjects.at( id ) );
	    return;
	}
	openProject( *recentlyProjects.at( id ) );
	QString fn( *recentlyProjects.at( id ) );
	addRecentlyOpened( fn, recentlyProjects );
    }
}

void MainWindow::addRecentlyOpened( const QString &fn, QStringList &lst )
{
    QFileInfo fi( fn );
    fi.convertToAbs();
    QString f = fi.filePath();
    if ( lst.find( f ) != lst.end() )
	lst.remove( f );
    if ( lst.count() >= 10 )
	lst.pop_back();
    lst.prepend( f );
}

TemplateWizardInterface * MainWindow::templateWizardInterface( const QString& className )
{
    TemplateWizardInterface* iface = 0;
    templateWizardPluginManager->queryInterface( className, & iface );
    return iface;
}

void MainWindow::setupPluginManagers()
{
    editorPluginManager = new QPluginManager<EditorInterface>( IID_Editor, QApplication::libraryPaths(), "/kdevdesigner" );
    MetaDataBase::setEditor( editorPluginManager->featureList() );

    templateWizardPluginManager =
	new QPluginManager<TemplateWizardInterface>( IID_TemplateWizard, QApplication::libraryPaths(), pluginDirectory() );

    MetaDataBase::setupInterfaceManagers( "/designer" );
    preferencePluginManager =
	new QPluginManager<PreferenceInterface>( IID_Preference, QApplication::libraryPaths(), pluginDirectory() );
    projectSettingsPluginManager =
	new QPluginManager<ProjectSettingsInterface>( IID_ProjectSettings, QApplication::libraryPaths(), pluginDirectory() );
    sourceTemplatePluginManager =
	new QPluginManager<SourceTemplateInterface>( IID_SourceTemplate, QApplication::libraryPaths(), pluginDirectory() );

    if ( preferencePluginManager ) {
	QStringList lst = preferencePluginManager->featureList();
	for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
	    PreferenceInterface *i = 0;
	    preferencePluginManager->queryInterface( *it, &i );
	    if ( !i )
		continue;
	    i->connectTo( designerInterface() );
	    PreferenceInterface::Preference *pf = i->preference();
	    if ( pf )
		addPreferencesTab( pf->tab, pf->title, pf->receiver, pf->init_slot, pf->accept_slot );
	    i->deletePreferenceObject( pf );

	    i->release();
	}
    }
    if ( projectSettingsPluginManager ) {
	QStringList lst = projectSettingsPluginManager->featureList();
	for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
	    ProjectSettingsInterface *i = 0;
	    projectSettingsPluginManager->queryInterface( *it, &i );
	    if ( !i )
		continue;
	    i->connectTo( designerInterface() );

	    ProjectSettingsInterface::ProjectSettings *pf = i->projectSetting();
	    if ( pf )
		addProjectTab( pf->tab, pf->title, pf->receiver, pf->init_slot, pf->accept_slot );
	    i->deleteProjectSettingsObject( pf );
	    i->release();
	}
    }
}

void MainWindow::addPreferencesTab( QWidget *tab, const QString &title, QObject *receiver, const char *init_slot, const char *accept_slot )
{
    Tab t;
    t.w = tab;
    t.title = title;
    t.receiver = receiver;
    t.init_slot = init_slot;
    t.accept_slot = accept_slot;
    preferenceTabs << t;
}

void MainWindow::addProjectTab( QWidget *tab, const QString &title, QObject *receiver, const char *init_slot, const char *accept_slot )
{
    Tab t;
    t.w = tab;
    t.title = title;
    t.receiver = receiver;
    t.init_slot = init_slot;
    t.accept_slot = accept_slot;
    projectTabs << t;
}

void MainWindow::setModified( bool b, QWidget *window )
{
    QWidget *w = window;
    while ( w ) {
	if ( ::qt_cast<FormWindow*>(w) ) {
	    ( (FormWindow*)w )->modificationChanged( b );
	    return;
	} else if ( ::qt_cast<SourceEditor*>(w) ) {
	    FormWindow *fw = ( (SourceEditor*)w )->formWindow();
	    if ( fw && !fw->isFake() ) {
		//fw->commandHistory()->setModified( b );
		//fw->modificationChanged( b );
		fw->formFile()->setModified( b, FormFile::WFormCode );
		wspace->update( fw->formFile() );
	    } else {
		wspace->update();
	    }
	    return;
	}
	w = w->parentWidget( TRUE );
    }
}

void MainWindow::editorClosed( SourceEditor *e )
{
    sourceEditors.take( sourceEditors.findRef( e ) );
}

void MainWindow::functionsChanged()
{
    updateFunctionsTimer->start( 0, TRUE );
}

void MainWindow::doFunctionsChanged()
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() )
	e->refresh( FALSE );
    hierarchyView->formDefinitionView()->refresh();
}

void MainWindow::updateFunctionList()
{
    if ( !qWorkspace()->activeWindow() || !::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) )
	return;
    SourceEditor *se = (SourceEditor*)qWorkspace()->activeWindow();
    se->save();
    hierarchyView->formDefinitionView()->refresh();
    if ( !currentProject->isCpp() && se->formWindow() ) {
	LanguageInterface *iface = MetaDataBase::languageInterface( currentProject->language() );
	if ( !iface )
	    return;
	QValueList<LanguageInterface::Connection> conns;
	iface->connections( se->text(), &conns );
	MetaDataBase::setupConnections( se->formWindow(), conns );
	propertyEditor->eventList()->setup();
    }
}

void MainWindow::updateWorkspace()
{
    wspace->setCurrentProject( currentProject );
}

void MainWindow::showDebugStep( QObject *o, int line )
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() )
	e->clearStep();
    if ( !o || line == -1 )
	return;
    showSourceLine( o, line, Step );
}

void MainWindow::showStackFrame( QObject *o, int line )
{
    if ( !o || line == -1 )
	return;
    showSourceLine( o, line, StackFrame );
}

void MainWindow::showErrorMessage( QObject *o, int errorLine, const QString &errorMessage )
{
    if ( o ) {
	errorLine--; // ######
	QValueList<uint> l;
	l << ( errorLine + 1 );
	QStringList l2;
	l2 << errorMessage;
	QObjectList ol;
	ol.append( o );
	QStringList ll;
	ll << currentProject->locationOfObject( o );
	oWindow->setErrorMessages( l2, l, TRUE, ll, ol );
	showSourceLine( o, errorLine, Error );
    }
}

void MainWindow::finishedRun()
{
    inDebugMode = FALSE;
    previewing = FALSE;
    debuggingForms.clear();
    enableAll( TRUE );
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->project() == currentProject )
	    e->editorInterface()->setMode( EditorInterface::Editing );
	e->clearStackFrame();
    }
    outputWindow()->clearErrorMessages();
}

void MainWindow::enableAll( bool enable )
{
    menuBar()->setEnabled( enable );
    QObjectList *l = queryList( "QDockWindow" );
    for ( QObject *o = l->first(); o; o = l->next() ) {
	if ( o == wspace->parentWidget() ||
	     o == oWindow->parentWidget() ||
	     o == hierarchyView->parentWidget() )
	    continue;
	( (QWidget*)o )->setEnabled( enable );
    }
    delete l;
}

void MainWindow::showSourceLine( QObject *o, int line, LineMode lm )
{
    QWidgetList windows = qworkspace->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	FormWindow *fw = 0;
	SourceEditor *se = 0;
	SourceFile *sf = 0;
	if ( ::qt_cast<FormWindow*>(w) ) {
	    fw = (FormWindow*)w;
	} else if ( ::qt_cast<SourceEditor*>(w) ) {
	    se = (SourceEditor*)w;
	    if ( !se->object() )
		continue;
	    if ( se->formWindow() )
		fw = se->formWindow();
	    else
		sf = se->sourceFile();
	}

	if ( fw ) {
	    if ( fw->project() != currentProject )
		continue;
	    if ( qstrcmp( fw->name(), o->name() ) == 0 ||
		 fw->isFake() && currentProject->objectForFakeForm( fw ) == o ) {
		if ( se ) {
		    switch ( lm ) {
		    case Error:
			se->editorInterface()->setError( line );
			break;
		    case Step:
			se->editorInterface()->setStep( line );
			break;
		    case StackFrame:
			se->editorInterface()->setStackFrame( line );
			break;
		    }
		    return;
		} else {
		    fw->showNormal();
		    fw->setFocus();
		    lastActiveFormWindow = fw;
		    qApp->processEvents();
		    se = editSource();
		    if ( se ) {
			switch ( lm ) {
			case Error:
			    se->editorInterface()->setError( line );
			    break;
			case Step:
			    se->editorInterface()->setStep( line );
			    break;
			case StackFrame:
			    se->editorInterface()->setStackFrame( line );
			    break;
			}
			return;
		    }
		}
	    }
	} else if ( se ) {
	    if ( o != sf )
		continue;
	    switch ( lm ) {
	    case Error:
		se->editorInterface()->setError( line );
		break;
	    case Step:
		se->editorInterface()->setStep( line );
		break;
	    case StackFrame:
		se->editorInterface()->setStackFrame( line );
		break;
	    }
	    return;
	}
    }

    if ( ::qt_cast<SourceFile*>(o) ) {
	for ( QPtrListIterator<SourceFile> sources = currentProject->sourceFiles();
	      sources.current(); ++sources ) {
	    SourceFile* f = sources.current();
	    if ( f == o ) {
		SourceEditor *se = editSource( f );
		if ( se ) {
		    switch ( lm ) {
		    case Error:
			se->editorInterface()->setError( line );
			break;
		    case Step:
			se->editorInterface()->setStep( line );
			break;
		    case StackFrame:
			se->editorInterface()->setStackFrame( line );
			break;
		    }
		}
		return;
	    }
	}
    }

    FormFile *ff = currentProject->fakeFormFileFor( o );
    FormWindow *fw = 0;
    if ( ff )
	fw = ff->formWindow();

    if ( !fw && !qwf_forms ) {
	qWarning( "MainWindow::showSourceLine: qwf_forms is NULL!" );
	return;
    }

    mblockNewForms = TRUE;
    if ( !fw )
	openFormWindow( currentProject->makeAbsolute( *qwf_forms->find( (QWidget*)o ) ) );
    else
	fw->formFile()->showEditor( FALSE );
    qApp->processEvents(); // give all views the chance to get the formwindow
    SourceEditor *se = editSource();
    if ( se ) {
	switch ( lm ) {
	case Error:
	    se->editorInterface()->setError( line );
	    break;
	case Step:
	    se->editorInterface()->setStep( line );
	    break;
	case StackFrame:
	    se->editorInterface()->setStackFrame( line );
	    break;
	}
    }
    mblockNewForms = FALSE;
}


QObject *MainWindow::findRealObject( QObject *o )
{
    QWidgetList windows = qWorkspace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( ::qt_cast<FormWindow*>(w) && QString( w->name() ) == QString( o->name() ) )
	    return w;
	else if ( ::qt_cast<SourceEditor*>(w) && ( (SourceEditor*)w )->formWindow() &&
		  QString( ( (SourceEditor*)w )->formWindow()->name() ) == QString( o->name() ) )
	    return w;
	else if ( ::qt_cast<SourceFile*>(w) && ( (SourceEditor*)w )->sourceFile() &&
		  ( (SourceEditor*)w )->sourceFile() == o )
	    return o;
    }
    return 0;
}

void MainWindow::formNameChanged( FormWindow *fw )
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->object() == fw )
	    e->refresh( TRUE );
	if ( e->project() == fw->project() )
	    e->resetContext();
    }
}

void MainWindow::breakPointsChanged()
{
    if ( !inDebugMode )
	return;
    if ( !qWorkspace()->activeWindow() || !::qt_cast<SourceEditor*>(qWorkspace()->activeWindow()) )
	return;
    SourceEditor *e = (SourceEditor*)qWorkspace()->activeWindow();
    if ( !e->object() || !e->project() )
	return;
    if ( e->project() != currentProject )
	return;

    if ( !interpreterPluginManager ) {
	interpreterPluginManager =
	    new QPluginManager<InterpreterInterface>( IID_Interpreter,
						      QApplication::libraryPaths(),
						      "/qsa" );
    }

    InterpreterInterface *iiface = 0;
    if ( interpreterPluginManager ) {
	QString lang = currentProject->language();
	iiface = 0;
	interpreterPluginManager->queryInterface( lang, &iiface );
	if ( !iiface )
	    return;
    }

    e->saveBreakPoints();

    for ( QObject *o = debuggingForms.first(); o; o = debuggingForms.next() ) {
	if ( qstrcmp( o->name(), e->object()->name() ) == 0 ) {
	    iiface->setBreakPoints( o, MetaDataBase::breakPoints( e->object() ) );
	    break;
	}
    }

    for ( e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->project() == currentProject && e->sourceFile() ) {
	    QValueList<uint> bps = MetaDataBase::breakPoints( e->sourceFile() );
	    iiface->setBreakPoints( e->object(), bps );
	}
    }

    iiface->release();
}

int MainWindow::currentLayoutDefaultSpacing() const
{
    if ( ( (MainWindow*)this )->formWindow() )
	return ( (MainWindow*)this )->formWindow()->layoutDefaultSpacing();
    return BOXLAYOUT_DEFAULT_SPACING;
}

int MainWindow::currentLayoutDefaultMargin() const
{
    if ( ( (MainWindow*)this )->formWindow() )
	return ( (MainWindow*)this )->formWindow()->layoutDefaultMargin();
    return BOXLAYOUT_DEFAULT_MARGIN;
}

void MainWindow::saveAllBreakPoints()
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	e->save();
	e->saveBreakPoints();
    }
}

void MainWindow::resetBreakPoints()
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() )
	e->resetBreakPoints();
}

SourceFile *MainWindow::sourceFile()
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( qworkspace->activeWindow() == e ) {
	    if ( e->sourceFile() )
		return e->sourceFile();
	}
    }
    return 0;
}

bool MainWindow::openProjectSettings( Project *pro )
{
    ProjectSettings dia( pro, this, 0, TRUE );
    SenderObject *senderObject = new SenderObject( designerInterface() );
    QValueList<Tab>::ConstIterator it;
    for ( it = projectTabs.begin(); it != projectTabs.end(); ++it ) {
	Tab t = *it;
	if ( t.title != pro->language() )
	    continue;
	dia.tabWidget->addTab( t.w, t.title );
	if ( t.receiver ) {
	    connect( dia.buttonOk, SIGNAL( clicked() ), senderObject, SLOT( emitAcceptSignal() ) );
	    connect( senderObject, SIGNAL( acceptSignal( QUnknownInterface * ) ), t.receiver, t.accept_slot );
	    connect( senderObject, SIGNAL( initSignal( QUnknownInterface * ) ), t.receiver, t.init_slot );
	    senderObject->emitInitSignal();
	    disconnect( senderObject, SIGNAL( initSignal( QUnknownInterface * ) ), t.receiver, t.init_slot );
	}
    }

    if ( singleProject )
	dia.tabWidget->setTabEnabled( dia.tabSettings, FALSE );

    int res = dia.exec();

    delete senderObject;

    for ( it = projectTabs.begin(); it != projectTabs.end(); ++it ) {
	Tab t = *it;
	dia.tabWidget->removePage( t.w );
	t.w->reparent( 0, QPoint(0,0), FALSE );
    }

    return res == QDialog::Accepted;
}

void MainWindow::popupProjectMenu( const QPoint &pos )
{
    projectMenu->exec( pos );
}

QStringList MainWindow::sourceTemplates() const
{
    return sourceTemplatePluginManager->featureList();
}

SourceTemplateInterface* MainWindow::sourceTemplateInterface( const QString& templ )
{
    SourceTemplateInterface *iface = 0;
    sourceTemplatePluginManager->queryInterface( templ, &iface);
    return iface;
}

QString MainWindow::whatsThisFrom( const QString &key )
{
    if ( menuHelpFile.isEmpty() ) {
	QString fn( documentationPath() );
	fn += "/designer-manual-11.html";
	QFile f( fn );
	if ( f.open( IO_ReadOnly ) ) {
	    QTextStream ts( &f );
	    menuHelpFile = ts.read();
	}
    }

    int i = menuHelpFile.find( key );
    if ( i == -1 )
	return QString::null;
    int start = i;
    int end = i;
    start = menuHelpFile.findRev( "<li>", i ) + 4;
    end = menuHelpFile.find( '\n', i ) - 1;
    return menuHelpFile.mid( start, end - start + 1 );
}

void MainWindow::setSingleProject( Project *pro )
{
    if ( eProject ) {
	Project *pro = eProject;
	pro->save();
	QWidgetList windows = qWorkspace()->windowList();
	qWorkspace()->blockSignals( TRUE );
	QWidgetListIt wit( windows );
	while ( wit.current() ) {
	    QWidget *w = wit.current();
	    ++wit;
	    if ( ::qt_cast<FormWindow*>(w) ) {
		if ( ( (FormWindow*)w )->project() == pro ) {
		    if ( ( (FormWindow*)w )->formFile()->editor() )
			windows.removeRef( ( (FormWindow*)w )->formFile()->editor() );
		    ( (FormWindow*)w )->formFile()->close();
		}
	    } else if ( ::qt_cast<SourceEditor*>(w) ) {
		( (SourceEditor*)w )->close();
	    }
	}
	hierarchyView->clear();
	windows = qWorkspace()->windowList();
	qWorkspace()->blockSignals( FALSE );
	currentProject = 0;
	updateUndoRedo( FALSE, FALSE, QString::null, QString::null );
    }

    singleProject = TRUE;
    projects.clear();
    QAction *a = new QAction( tr( pro->name() ), tr( pro->name() ), 0,
			      actionGroupProjects, 0, TRUE );
    eProject = pro;
    projects.insert( a, eProject );
    a->setOn( TRUE );
    actionGroupProjects->removeFrom( projectMenu );
    actionGroupProjects->removeFrom( projectToolBar );
    currentProject = eProject;
    currentProject->designerCreated();
}

void MainWindow::shuttingDown()
{
    outputWindow()->shuttingDown();
}

void MainWindow::showGUIStuff( bool b )
{
    if ( (bool)guiStuffVisible == b )
	return;
    guiStuffVisible = b;
    if ( !b ) {
	setAppropriate( (QDockWindow*)toolBox->parentWidget(), FALSE );
	toolBox->parentWidget()->hide();
	for ( QToolBar *tb = widgetToolBars.first(); tb; tb = widgetToolBars.next() ) {
	    tb->hide();
	    setAppropriate( tb, FALSE );
	}
	propertyEditor->setPropertyEditorEnabled( FALSE );
	setAppropriate( layoutToolBar, FALSE );
	layoutToolBar->hide();
	setAppropriate( toolsToolBar, FALSE );
	toolsToolBar->hide();
	menubar->removeItem( toolsMenuId );
	menubar->removeItem( toolsMenuId + 1 );
	menubar->removeItem( toolsMenuId + 2 );
	disconnect( this, SIGNAL( hasActiveForm(bool) ), actionEditAccels, SLOT( setEnabled(bool) ) );
	disconnect( this, SIGNAL( hasActiveForm(bool) ), actionEditFunctions, SLOT( setEnabled(bool) ) );
	disconnect( this, SIGNAL( hasActiveForm(bool) ), actionEditConnections, SLOT( setEnabled(bool) ) );
	disconnect( this, SIGNAL( hasActiveForm(bool) ), actionEditSource, SLOT( setEnabled(bool) ) );
	disconnect( this, SIGNAL( hasActiveForm(bool) ), actionEditFormSettings, SLOT( setEnabled(bool) ) );
	actionEditFormSettings->setEnabled( FALSE );
	actionEditSource->setEnabled( FALSE );
	actionEditConnections->setEnabled( FALSE );
	actionEditFunctions->setEnabled( FALSE );
	actionEditAccels->setEnabled( FALSE );
	( (QDockWindow*)propertyEditor->parentWidget() )->
	    setCaption( tr( "Signal Handlers" ) );
	actionGroupNew->removeFrom( fileMenu );
	actionGroupNew->removeFrom( projectToolBar );
	actionFileSave->removeFrom( fileMenu );
	actionFileSave->removeFrom( projectToolBar );
	actionFileExit->removeFrom( fileMenu );
	actionNewFile->addTo( fileMenu );
	actionNewFile->addTo( projectToolBar );
	actionFileSave->addTo( fileMenu );
	actionFileSave->addTo( projectToolBar );
	actionFileExit->addTo( fileMenu );
    } else {
	setAppropriate( (QDockWindow*)toolBox->parentWidget(), TRUE );
	toolBox->parentWidget()->show();
	for ( QToolBar *tb = widgetToolBars.first(); tb; tb = widgetToolBars.next() ) {
	    setAppropriate( tb, TRUE );
	    tb->hide();
	}
	propertyEditor->setPropertyEditorEnabled( TRUE );
	setAppropriate( layoutToolBar, TRUE );
	layoutToolBar->show();
	setAppropriate( toolsToolBar, TRUE );
	toolsToolBar->show();
	menubar->insertItem( tr( "&Tools" ), toolsMenu, toolsMenuId, toolsMenuIndex );
	menubar->insertItem( tr( "&Layout" ), layoutMenu, toolsMenuId + 1, toolsMenuIndex + 1 );
	menubar->insertItem( tr( "&Preview" ), previewMenu, toolsMenuId + 2, toolsMenuIndex + 2 );
	connect( this, SIGNAL( hasActiveForm(bool) ), actionEditAccels, SLOT( setEnabled(bool) ) );
	connect( this, SIGNAL( hasActiveForm(bool) ), actionEditFunctions, SLOT( setEnabled(bool) ) );
	connect( this, SIGNAL( hasActiveForm(bool) ), actionEditConnections, SLOT( setEnabled(bool) ) );
	connect( this, SIGNAL( hasActiveForm(bool) ), actionEditSource, SLOT( setEnabled(bool) ) );
	connect( this, SIGNAL( hasActiveForm(bool) ), actionEditFormSettings, SLOT( setEnabled(bool) ) );
	actionEditFormSettings->setEnabled( TRUE );
	actionEditSource->setEnabled( TRUE );
	actionEditConnections->setEnabled( TRUE );
	actionEditFunctions->setEnabled( TRUE );
	actionEditAccels->setEnabled( TRUE );
	( (QDockWindow*)propertyEditor->parentWidget() )->
	    setCaption( tr( "Property Editor/Signal Handlers" ) );
	actionFileSave->removeFrom( fileMenu );
	actionFileSave->removeFrom( projectToolBar );
	actionFileExit->removeFrom( fileMenu );
	actionGroupNew->addTo( fileMenu );
	actionGroupNew->addTo( projectToolBar );
	actionFileSave->addTo( fileMenu );
	actionFileSave->addTo( projectToolBar );
	actionFileExit->addTo( fileMenu );
    }
}

void MainWindow::setEditorsReadOnly( bool b )
{
    editorsReadOnly = b;
}

void MainWindow::setPluginDirectory( const QString &pd )
{
    pluginDir = pd;
    if ( !qwf_plugin_dir )
	qwf_plugin_dir = new QString( pd );
    else
	*qwf_plugin_dir = pd;
}

void MainWindow::toggleSignalHandlers( bool show )
{
    if ( sSignalHandlers == show )
	return;
    sSignalHandlers = show;
    propertyEditor->setSignalHandlersEnabled( show );
}

void MainWindow::statusMessage( const QString &msg )
{
    m_part->statusMessage(msg);
}

/***************************************************************************
 newmainwindow.cpp
                             -------------------
    begin                : october 2003
    copyright            : (C) 2003, 2004 by the KDevelop team
    email                : team@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <qlayout.h>
#include <qmultilineedit.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qvaluelist.h>
#include <qobjectlist.h>
#include <qtabbar.h>

#include <kdeversion.h>
#include <kapplication.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <kaction.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kconfig.h>
#include <kstatusbar.h>
#include <kdialogbase.h>
#include <kkeydialog.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <kdevproject.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kmultitabbar.h>
#include <ktabwidget.h>
#include <kparts/part.h>

#if (KDE_VERSION > 305)
#include <knotifydialog.h>
#endif

#include <kedittoolbar.h>

#include "kdevplugin.h"

#include "projectmanager.h"
#include "plugincontroller.h"
#include "partcontroller.h"
#include "kdevcore.h"
#include "kdevpartcontroller.h"
#include "partselectwidget.h"
#include "api.h"
#include "core.h"
#include "settingswidget.h"
#include "statusbar.h"
#include "kpopupmenu.h"
#include "documentationpart.h"

#include "toplevel.h"
#include "mainwindowshare.h"
#include "newmainwindow.h"

/***************************
WHAT'S NOT WORKING:

The XMLGUI-created 'window' menu is overwritten by kmdi's menu. What's needed is a way
to make these merge properly. The best way would probably be to make KMdiMainFrm XMLGUI-aware.

Toolview widgets without a name doesn't show in the side toolbars. (Port safety method from old mainwindow)
Culprits are: debugger views, problem reporter, more???

UI mode can now only be changed via a restart. This is intentional!


****************************/

NewMainWindow::NewMainWindow(QWidget *parent, const char *name, KMdi::MdiMode mdimode)
 : KMdiMainFrm(parent, name, mdimode )
// ,m_pWindowMenu(0L)
// ,m_bSwitching(false)
{
    resize( 800, 600 ); // starts kdevelop at 800x600 the first time
  
	// isn't it fun with stuff you have no idea what it does?
    this->setManagedDockPositionModeEnabled(true);
	
	// does this work at all?
    this->setStandardMDIMenuEnabled( false );

    KConfig *config = kapp->config();
    config->setGroup("UI");
    int mdiStyle = config->readNumEntry("MDIStyle", 1);

    this->setIDEAlModeStyle(mdiStyle); // KDEV3 style of KMultiTabBar

    
    m_pMainWindowShare = new MainWindowShare(this);
    
	m_raiseEditor = new KAction( i18n("Raise &Editor"), ALT+Key_C,
                                 this, SLOT(raiseEditor()),
                                 actionCollection(), "raise_editor");
    m_raiseEditor->setToolTip(i18n("Raise editor"));
    m_raiseEditor->setWhatsThis(i18n("<b>Raise editor</b><p>Focuses the editor."));
	
	//@fixme why is this part of KDevMainWindow?
//    previous_output_view = NULL;
}


void NewMainWindow::init() {

    actionCollection()->setHighlightingEnabled( true );
//    m_windowDynamicMenus.setAutoDelete(true);

    setStandardToolBarMenuEnabled( true );

    if (TopLevel::mode == TopLevel::AssistantMode)
        setXMLFile("kdevassistantui.rc");
    else
        setXMLFile("kdevelopui.rc");

    createFramework();
    createActions();
    createStatusBar();

    createGUI(0);
    
    readDockConfig(kapp->config());

    m_pMainWindowShare->init();
	
	// remove the kmdi-created menu
	delete m_pWindowMenu;
	
	// get the xmlgui created one instead
    m_pWindowMenu = static_cast<QPopupMenu*>(main()->child( "window", "KPopupMenu" ));

    if( !m_pWindowMenu )
	{
		kdDebug(9000) << "Couldn't find the XMLGUI window menu. Creating new." << endl;
		
		m_pWindowMenu = new QPopupMenu( main(), "window");
		menuBar()->insertItem(i18n("&Window"),m_pWindowMenu);
	}
	
    m_pWindowMenu->setCheckable( TRUE);
	QObject::connect( m_pWindowMenu, SIGNAL(activated(int)), this, SLOT(openURL(int )) );
    QObject::connect( m_pWindowMenu, SIGNAL(aboutToShow()), this, SLOT(fillWindowMenu()) );
			   	   
	menuBar()->setEnabled( false );
	
    if ( PluginController::pluginServices().isEmpty() ) {
        KMessageBox::sorry( this, i18n("Unable to find plugins, KDevelop will not work properly!\nPlease make sure "
                                       "that KDevelop is installed in your KDE directory, otherwise you have to add KDevelop's installation "
                                       "path to the environment variable KDEDIRS and run kbuildsycoca. Restart KDevelop afterwards.\n"
                                       "Example for BASH users:\nexport KDEDIRS=/path/to/kdevelop:$KDEDIRS && kbuildsycoca"),
                            i18n("Could Not Find Plugins") );
    }
	
	connect( Core::getInstance(), SIGNAL(coreInitialized()), this, SLOT(slotCoreInitialized()) );
	
	connect( PartController::getInstance(), SIGNAL(partURLChanged(KParts::ReadOnlyPart * )),
		this, SLOT(slotPartURLChanged(KParts::ReadOnlyPart * )) );
	
	connect( PartController::getInstance(), SIGNAL(documentChangedState(const KURL &, DocumentState)), 
		this, SLOT(documentChangedState(const KURL&, DocumentState )) );
	   
#if KDE_VERSION >= KDE_MAKE_VERSION(3,2,90)
	if ( tabWidget() )
	{
		KConfig *config = kapp->config();
		config->setGroup("UI");
		
		int tabvisibility = config->readNumEntry( "TabWidgetVisibility", KMdi::AlwaysShowTabs );
		setTabWidgetVisibility( (KMdi::TabWidgetVisibility)tabvisibility );
		
		bool CloseOnHover = config->readBoolEntry( "CloseOnHover", false );
		tabWidget()->setHoverCloseButton( CloseOnHover );
		
		bool CloseOnHoverDelay = config->readBoolEntry( "CloseOnHoverDelay", false );
		tabWidget()->setHoverCloseButtonDelayed( CloseOnHoverDelay );
	}
#endif
}

NewMainWindow::~NewMainWindow() {
    TopLevel::invalidateInstance( this );
}

void NewMainWindow::slotCoreInitialized( )
{
	menuBar()->setEnabled( true );
}

void NewMainWindow::openURL( int id )
{
	QValueList< QPair< int, KURL > >::ConstIterator it = m_windowList.begin();
	while ( it != m_windowList.end() )
	{
		if ( (*it).first == id )
		{
			KURL url( (*it).second );
			if ( !url.isEmpty() )
			{
				PartController::getInstance()->activatePart( PartController::getInstance()->partForURL( url ) );
				return;
			}
		}
		++it;
	}
}

void NewMainWindow::fillWindowMenu()
{
	bool hasWidget = ( PartController::getInstance()->activeWidget() != 0 );	// hmmm... works??
	
	// clear menu
	QValueList< QPair< int, KURL > >::ConstIterator it = m_windowList.begin();
	while ( it != m_windowList.end() )
	{
		m_pWindowMenu->removeItem( (*it).first );
		++it;
	}
	
	int temp = 0;
	
	temp = m_pWindowMenu->insertItem(i18n("&Close"), PartController::getInstance(), SLOT(slotCloseWindow()));
	m_pWindowMenu->setItemEnabled( temp, hasWidget );
	m_windowList << qMakePair( temp, KURL() );
	
	temp = m_pWindowMenu->insertItem(i18n("Close &All"), PartController::getInstance(), SLOT(slotCloseAllWindows()));
	m_pWindowMenu->setItemEnabled( temp, hasWidget );
	m_windowList << qMakePair( temp, KURL() );
	
	temp = m_pWindowMenu->insertItem(i18n("Close All &Others"), PartController::getInstance(), SLOT(slotCloseOtherWindows()));
	m_pWindowMenu->setItemEnabled( temp, hasWidget );
	m_windowList << qMakePair( temp, KURL() );
	
	if	( m_mdiMode == KMdi::ChildframeMode )
	{
		temp = m_pWindowMenu->insertItem(i18n("&Minimize All"), this, SLOT(iconifyAllViews()));
		m_pWindowMenu->setItemEnabled( temp, hasWidget );
		m_windowList << qMakePair( temp, KURL() );
		
		m_windowList << qMakePair( m_pWindowMenu->insertSeparator(), KURL() );
		
		temp = m_pWindowMenu->insertItem(i18n("&Tile..."), m_pPlacingMenu);
		m_windowList << qMakePair( temp, KURL() );
		m_pWindowMenu->setItemEnabled( temp, hasWidget );
	
		m_pPlacingMenu->clear();
		m_pPlacingMenu->insertItem(i18n("Ca&scade Windows"), m_pMdi,SLOT(cascadeWindows()));
		m_pPlacingMenu->insertItem(i18n("Cascade &Maximized"), m_pMdi,SLOT(cascadeMaximized()));
		m_pPlacingMenu->insertItem(i18n("Expand &Vertically"), m_pMdi,SLOT(expandVertical()));
		m_pPlacingMenu->insertItem(i18n("Expand &Horizontally"), m_pMdi,SLOT(expandHorizontal()));
		m_pPlacingMenu->insertItem(i18n("Tile &Non-overlapped"), m_pMdi,SLOT(tileAnodine()));
		m_pPlacingMenu->insertItem(i18n("Tile Overla&pped"), m_pMdi,SLOT(tilePragma()));
		m_pPlacingMenu->insertItem(i18n("Tile V&ertically"), m_pMdi,SLOT(tileVertically()));
	}

	m_windowList << qMakePair( m_pWindowMenu->insertSeparator(), KURL() );
	
	KURL::List list = PartController::getInstance()->openURLs();
	KURL::List::Iterator itt = list.begin();
	while ( itt != list.end() )
	{
		temp = m_pWindowMenu->insertItem( (*itt).fileName() );
		m_windowList << qMakePair( temp, *itt );
		
		++itt;
	}
}

bool NewMainWindow::queryClose()
{
    saveSettings(); //moved from queryClose so tab settings can be saved while
                    //the components still exist.
    return Core::getInstance()->queryClose();
}

bool NewMainWindow::queryExit()
{
  return true;
}

KMainWindow *NewMainWindow::main() {
    return this;
}


void NewMainWindow::createStatusBar() {
    (void) new StatusBar(this);
//    QMainWindow::statusBar();
}


void NewMainWindow::createFramework() {

    PartController::createInstance( this );

    connect(this, SIGNAL(viewActivated(KMdiChildView*)), this, SLOT(slotViewActivated(KMdiChildView*)) );
    connect(this, SIGNAL(currentChanged(QWidget*)), PartController::getInstance(), SLOT(slotCurrentChanged(QWidget*)));
    connect(this, SIGNAL(sigCloseWindow(const QWidget *)), PartController::getInstance(),SLOT(slotClosePartForWidget(const QWidget *)));
    connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(createGUI(KParts::Part*)));
}

void NewMainWindow::slotViewActivated(KMdiChildView* child)
{
    kdDebug(9000) << "======> view activated: " << child << endl;
    if( !child || !child->focusedChildWidget() )
        return;
    
    emit currentChanged( child->focusedChildWidget() );
}

void NewMainWindow::createActions() 
{
    m_pMainWindowShare->createActions();

    connect(m_pMainWindowShare, SIGNAL(gotoNextWindow()), this, SLOT(activateNextWin()) );
    connect(m_pMainWindowShare, SIGNAL(gotoPreviousWindow()), this, SLOT(activatePrevWin()) );
    connect(m_pMainWindowShare, SIGNAL(gotoFirstWindow()), this, SLOT(activateFirstWin()) );
    connect(m_pMainWindowShare, SIGNAL(gotoLastWindow()), this, SLOT(activateLastWin()) );
}

void NewMainWindow::embedPartView(QWidget *view, const QString &name, const QString& ) 
{
	if( !view ) return;

    QString shortName = name;
    shortName = shortName.right( shortName.length() - (shortName.findRev('/') +1));

    KMdiChildView * child = createWrapper(view, name, shortName);
    
    const QPixmap* wndIcon = view->icon();
    if (!wndIcon || (wndIcon && (wndIcon->size().height() > 16))) {
        child->setIcon(SmallIcon("kdevelop")); // was empty or too big, take something useful
    }

    unsigned int mdiFlags = KMdi::StandardAdd | KMdi::UseKMdiSizeHint;
    addWindow(child, mdiFlags);
}


void NewMainWindow::embedSelectView(QWidget *view, const QString &name, const QString &toolTip) 
{
	if( !view ) return;
	KMdiMainFrm::addToolWindow( view, KDockWidget::DockLeft, getMainDockWidget(), 20, toolTip, name );
}

void NewMainWindow::embedSelectViewRight ( QWidget* view, const QString& name, const QString &toolTip) 
{
	if( !view ) return;
	KMdiMainFrm::addToolWindow( view, KDockWidget::DockRight, getMainDockWidget(), 20, toolTip, name );
     if (TopLevel::mode == TopLevel::AssistantMode)
         raiseView(view);
}

void NewMainWindow::embedOutputView(QWidget *view, const QString &name, const QString &toolTip) 
{
	if( !view ) return;
	KMdiMainFrm::addToolWindow( view, KDockWidget::DockBottom, getMainDockWidget(), 20, toolTip, name );
}

void NewMainWindow::childWindowCloseRequest( KMdiChildView * childView )
{
	const QPtrList<KParts::Part> * partlist = PartController::getInstance()->parts();
	QPtrListIterator<KParts::Part> it( *partlist );
	while ( KParts::Part* part = it.current() )
	{
		if ( part->widget() && part->widget()->parentWidget() == childView )
		{
			PartController::getInstance()->closePart( part );
			return;
		}
		++it;
	}
}

void NewMainWindow::removeView( QWidget * view )
{
    kdDebug(9000) << k_funcinfo << " - view: " << view << endl;
    
	if( !view || !view->parentWidget() )
        return;
    
	kdDebug(9000) << "parentWidget: " << view->parentWidget() << endl;

	if( KMdiChildView * childView = static_cast<KMdiChildView*>(view->parentWidget()->qt_cast("KMdiChildView")) ) 
	{
		(void) view->reparent(0, QPoint(0,0), false );	// why?
		closeWindow( childView );
	} 
	else if( view->parentWidget()->qt_cast("KDockWidget") ) 
	{
		deleteToolWindow( view );
	}
}

void NewMainWindow::setViewAvailable(QWidget * /*pView*/, bool /*bEnabled*/) {
    /// @todo implement me
}

void NewMainWindow::raiseView(QWidget *view) 
{
	kdDebug(9000) << k_funcinfo << endl;
    
    if( !view || !view->parentWidget() )
        return;

    view->parentWidget()->setFocus();

    if( QGuardedPtr<KDockWidget> dockWidget = static_cast<KDockWidget*>(view->parentWidget()->qt_cast("KDockWidget")) ) {
        if( !dockWidget->isVisible() )
            makeDockVisible( dockWidget );
    }
}


void NewMainWindow::lowerView(QWidget *)
{
  // seems there's nothing to do here
}


void NewMainWindow::createGUI(KParts::Part *part) {
    if ( !part )
        setCaption( QString::null );
    KMdiMainFrm::createGUI(part);

    m_pMainWindowShare->slotGUICreated( part );
}


void NewMainWindow::loadSettings() {
    KConfig *config = kapp->config();

//	loadMDISettings();

    ProjectManager::getInstance()->loadSettings();
    applyMainWindowSettings(config, "Mainwindow");
}
/*
void NewMainWindow::loadMDISettings()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  int mdiMode = config->readNumEntry("MDIMode", KMdi::ChildframeMode);
  switch (mdiMode)
  {
  case KMdi::ToplevelMode:
    {
      int childFrmModeHt = config->readNumEntry("Childframe mode height", kapp->desktop()->height() - 50);
      resize(width(), childFrmModeHt);
      switchToToplevelMode();
    }
    break;

  case KMdi::ChildframeMode:
    break;

  case KMdi::TabPageMode:
    {
      int childFrmModeHt = config->readNumEntry("Childframe mode height", kapp->desktop()->height() - 50);
      resize(width(), childFrmModeHt);
      switchToTabPageMode();
    }
    break;
  case KMdi::IDEAlMode:
    {
      switchToIDEAlMode();
    }

  default:
    break;
  }

  // restore a possible maximized Childframe mode
  bool maxChildFrmMode = config->readBoolEntry("maximized childframes", true);
  setEnableMaximizedChildFrmMode(maxChildFrmMode);

  // teatime - what does this do??
  readDockConfig(0L, "dockSession_version1");
}
*/

void NewMainWindow::setUserInterfaceMode( const QString & )
{
/*	
    // immediately switch the mode likely set in the uimode part
    if (uiMode == "Childframe") {
	switchToChildframeMode();
    }
    else if (uiMode == "TabPage") {
	switchToTabPageMode();
    }
    else if (uiMode == "Toplevel") {
	switchToToplevelMode();
    }
    else if (uiMode == "KMDI-IDEAl") {
        switchToIDEAlMode();
    }
*/	
}

void NewMainWindow::saveSettings() 
{
    KConfig *config = kapp->config();

    ProjectManager::getInstance()->saveSettings();
    writeDockConfig(config);
    saveMainWindowSettings(config, "Mainwindow");
}

void NewMainWindow::raiseEditor( )
{
	kdDebug() << k_funcinfo << endl;
	
	KDevPartController * partcontroller = API::getInstance()->partController();
	if ( partcontroller->activePart() && partcontroller->activePart()->widget() )
	{
		partcontroller->activePart()->widget()->setFocus();
	}
}

void NewMainWindow::setCaption( const QString & caption )
{
	KDevProject * project = API::getInstance()->project();
	if ( project && !caption.isEmpty() )
	{
		QString projectname = project->projectName();

		QString suffix(".kdevelop");
		if ( projectname.endsWith( suffix ) )
		{
			projectname.truncate( projectname.length() - suffix.length() );
		}

		KMdiMainFrm::setCaption( projectname + " - " + caption );
	}
	else
	{
		KMdiMainFrm::setCaption( caption );
	}
}

//@fixme - not currently connected - should it be?
void NewMainWindow::projectOpened() 
{
	// why would we reload mainwindow settings on project load?
	loadSettings();
}

void NewMainWindow::prepareToCloseViews()
{
  // seems there's nothing to do here
}

void NewMainWindow::guiRestoringFinished()
{
  // seems there's nothing to do here
}

void NewMainWindow::storeOutputViewTab( )
{
  // seems there's nothing to do here
}

void NewMainWindow::restoreOutputViewTab( )
{
  // seems there's nothing to do here
}

void NewMainWindow::lowerAllViews( )
{
  // seems there's nothing to do here
}

void NewMainWindow::slotPartURLChanged( KParts::ReadOnlyPart * ro_part )
{
	kdDebug() << k_funcinfo << endl;
	
	if ( !ro_part || !ro_part->widget() || !ro_part->widget()->parentWidget() ) return;
	
	KMdiChildView * childView = dynamic_cast<KMdiChildView*>( ro_part->widget()->parentWidget() );
	if ( childView )
	{
		childView->setMDICaption( ro_part->url().fileName() );
	}
}

void NewMainWindow::documentChangedState( const KURL & url, DocumentState state )
{
	KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( PartController::getInstance()->partForURL( url ) );
	if ( ro_part && ro_part->widget() )
	{
		switch( state )
		{
			// we should probably restore the original icon instead of just using "kdevelop",
			// but I have never seen any other icon in use so this should do for now
			case Clean:
				ro_part->widget()->setIcon( SmallIcon("kdevelop"));
				break;
			case Modified:
				ro_part->widget()->setIcon( SmallIcon("filesave"));
				break;
			case Dirty:
				ro_part->widget()->setIcon( SmallIcon("revert"));
				break;
			case DirtyAndModified:
				ro_part->widget()->setIcon( SmallIcon("stop"));
				break;					
		}
	}
}

#include "newmainwindow.moc"

/***************************************************************************
 mainwindowideal.cpp  -  KDevelop main widget for IDEAl user interface mode
                             -------------------
    begin                : 22 Dec 2002
    copyright            : (C) 2002 by the KDevelop team
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

#if (KDE_VERSION > 305)
#include <knotifydialog.h>
#endif

#include <kedittoolbar.h>

#include "widgets/ktabzoomwidget.h"
#include "widgets/ktabwidget.h"
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


#include "toplevel.h"
#include "mainwindowshare.h"
#include "mainwindowideal.h"

#ifdef KeyRelease
#undef KeyRelease
#endif

class IDEAlEventFilter: public QObject
{
public:
    IDEAlEventFilter( MainWindowIDEAl* m, const char* name=0 )
	: QObject( m, name ), mw( m ) {}

    bool eventFilter( QObject * obj, QEvent *e )
    {
	if( e->type() == QEvent::FocusIn && mw->m_tabWidget->currentPage() )
	{
	    const QObjectList* l = mw->m_tabWidget->currentPage()->children();
	    if( l && l->contains(obj) ){
	      if( mw->m_leftBar->current() )
		  mw->m_leftBar->lowerWidget( mw->m_leftBar->current() );
	      if( mw->m_rightBar->current() )
		  mw->m_rightBar->lowerWidget( mw->m_rightBar->current() );
	      if( mw->m_bottomBar->current() )
		 mw->m_bottomBar->lowerWidget( mw->m_bottomBar->current() );
	    }
	}
	return FALSE;  // standard event processing
    }

private:
    MainWindowIDEAl* mw;
};


MainWindowIDEAl::MainWindowIDEAl(QWidget *parent, const char *name)
 : KParts::MainWindow(parent, name)
 ,m_pWindowMenu(0L)
 ,m_closing(false)
 ,m_bSwitching(false)
{
    resize( 800, 600 ); // starts kdevelop at 800x600 the first time
    m_pMainWindowShare = new MainWindowShare(this);

    m_raiseLeftBar = new KAction( i18n("Switch &Left Tabbar"), ALT+Key_L,
                                  this, SLOT(raiseLeftTabbar()),
                                  actionCollection(), "raise_left_tabbar");

    m_raiseRightBar = new KAction( i18n("Switch &Right Tabbar"), ALT+Key_R,
                                   this, SLOT(raiseRightTabbar()),
                                   actionCollection(), "raise_right_tabbar");

    m_raiseBottomBar = new KAction( i18n("Switch &Bottom Tabbar"), ALT+Key_U,
                                    this, SLOT(raiseBottomTabbar()),
                                    actionCollection(), "raise_bottom_tabbar");

    m_raiseEditor = new KAction( i18n("Raise &Editor"), ALT+Key_C,
                                 this, SLOT(raiseEditor()),
                                 actionCollection(), "raise_editor");

    m_raiseLeftBar->setEnabled( false );
    m_raiseRightBar->setEnabled( false );
    m_raiseBottomBar->setEnabled( false );

    m_timeStamps.clear();
}


void MainWindowIDEAl::init() {
#if (KDE_VERSION > 305)
    setStandardToolBarMenuEnabled( true );
#endif

    setXMLFile("gideonui.rc");

    createFramework();
    createActions();
    createStatusBar();

    createGUI(0);

    KAction *a = actionCollection()->action("help_report_bug");
    disconnect(a, SIGNAL(activated()), 0, 0);
    connect(a, SIGNAL(activated()), m_pMainWindowShare, SLOT(slotReportBug()));
    //actionCollection()->remove(a);
    if(a) qDebug("HI");

    m_pWindowMenu = (QPopupMenu*) main()->child( "window", "KPopupMenu" );

    if( !m_pWindowMenu ){
	// Add window menu to the menu bar
	m_pWindowMenu = new QPopupMenu( main(), "window");
	m_pWindowMenu->setCheckable( TRUE);
	menuBar()->insertItem(i18n("&Window"),m_pWindowMenu);
    }

    QObject::connect( m_pWindowMenu, SIGNAL(aboutToShow()), main(), SLOT(slotFillWindowMenu()) );


    slotFillWindowMenu();  // Just in case there is no file open. The menu would then be empty.

    if ( PluginController::pluginServices().isEmpty() ) {
        KMessageBox::sorry( this, i18n("Unable to find plugins, KDevelop won't work properly!\nPlease make sure "
                                       "that KDevelop is installed in your KDE directory, otherwise you have to add KDevelop's installation "
                                       "path to the environment variable KDEDIRS and run kbuildsycoca. Restart KDevelop afterwards.\n"
                                       "Example for BASH users:\nexport KDEDIRS=/path/to/gideon:$KDEDIRS && kbuildsycoca"),
                            i18n("Couldn't find plugins") );
    }
    kdDebug(9000) << "-> kapp = " << kapp << endl;
    kapp->installEventFilter( new IDEAlEventFilter(this) );
}

MainWindowIDEAl::~MainWindowIDEAl() {
    TopLevel::invalidateInstance( this );
    delete m_pWindowMenu;
}


bool MainWindowIDEAl::queryClose() {
    if (m_closing)
        return true;

    emit wantsToQuit();
    return false;
}

void MainWindowIDEAl::prepareToCloseViews()
{
  // seems there's nothing to do here
}

void MainWindowIDEAl::realClose()
{
  saveSettings();

  m_closing = true;
  close();
}


KMainWindow *MainWindowIDEAl::main() {
    return this;
}


void MainWindowIDEAl::createStatusBar() {
    (void) new StatusBar(this);
}


void MainWindowIDEAl::createFramework() {
    m_leftBar = new KTabZoomWidget(this, KTabZoomPosition::Left);
    setCentralWidget(m_leftBar);

    m_bottomBar = new KTabZoomWidget(m_leftBar, KTabZoomPosition::Bottom);
    m_leftBar->addContent(m_bottomBar);

    m_rightBar = new KTabZoomWidget ( m_bottomBar, KTabZoomPosition::Right );
    m_bottomBar->addContent ( m_rightBar );

    m_tabWidget = new KTabWidget(m_rightBar);
    m_tabWidget->setMargin(2);

    PartController::createInstance(m_tabWidget);

    m_bottomBar->addContent(m_tabWidget);
    m_rightBar->addContent (m_tabWidget );

    connect(m_tabWidget, SIGNAL(currentChanged(QWidget*)),
            PartController::getInstance(), SLOT(slotCurrentChanged(QWidget*)));
    connect(m_tabWidget, SIGNAL(currentChanged(QWidget*)), SLOT(slotTabSelected(QWidget*)));
    connect(m_tabWidget, SIGNAL(closeWindow(const QWidget *)), PartController::getInstance(),SLOT(slotClosePartForWidget(const QWidget *)));
    connect(m_tabWidget, SIGNAL(closeOthers(QWidget *)), PartController::getInstance(), SLOT(slotCloseAllButPartForWidget(QWidget *)));

    connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)),
            this, SLOT(createGUI(KParts::Part*)));

    connect( m_leftBar, SIGNAL(tabsChanged()), this, SLOT(slotLeftTabsChanged()) );
    connect( m_rightBar, SIGNAL(tabsChanged()), this, SLOT(slotRightTabsChanged()) );
    connect( m_bottomBar, SIGNAL(tabsChanged()), this, SLOT(slotBottomTabsChanged()) );

    connect(PartController::getInstance(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotPartAdded(KParts::Part*)));
    connect(PartController::getInstance(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotStatusChange(KParts::Part*)));
//    connect(PartController::getInstance(), SIGNAL(partRemoved(KParts::Part*)), this, SLOT(slotStatusChange(KParts::Part*)));
//    connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotStatusChange(KParts::Part*)));
    connect(PartController::getInstance(), SIGNAL(savedFile(const QString&)), this, SLOT(slotUpdateModifiedFlags()));
//     connect(PartController::getInstance(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotFillWindowMenu()));
//     connect(PartController::getInstance(), SIGNAL(partRemoved(KParts::Part*)), this, SLOT(slotFillWindowMenu()));
//     connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotFillWindowMenu()));
//     connect(PartController::getInstance(), SIGNAL(savedFile(const QString&)), this, SLOT(slotUpdateModifiedFlags()));
}



void MainWindowIDEAl::createActions() {
    m_pMainWindowShare->createActions();
    connect(m_pMainWindowShare, SIGNAL(gotoNextWindow()), this, SLOT(gotoNextWindow()));
    connect(m_pMainWindowShare, SIGNAL(gotoPreviousWindow()), this, SLOT(gotoPreviousWindow()));
    connect(m_pMainWindowShare, SIGNAL(gotoFirstWindow()), this, SLOT(gotoFirstWindow()));
    connect(m_pMainWindowShare, SIGNAL(gotoLastWindow()), this, SLOT(gotoLastWindow()));
}


void MainWindowIDEAl::embedPartView(QWidget *view, const QString &name, const QString& toolTip) {
    m_timeStamps[view]= QDateTime::currentDateTime();
    m_tabWidget->addTab(view, name);
    m_tabWidget->setTabToolTip(view, toolTip);
    m_tabWidget->showPage(view);
    view->installEventFilter( this );
}


void MainWindowIDEAl::embedSelectView(QWidget *view, const QString &name, const QString &toolTip) {
    m_leftBar->addTab(view, name, toolTip);
}

void MainWindowIDEAl::embedSelectViewRight ( QWidget* view, const QString& title, const QString &toolTip) {
    m_rightBar->addTab (view, title, toolTip);
}

void MainWindowIDEAl::embedOutputView(QWidget *view, const QString &name, const QString &toolTip) {
    m_bottomBar->addTab(view, name, toolTip);
}

void MainWindowIDEAl::removeView(QWidget *view)
{
    QMap<QWidget*, QDateTime>::iterator it( m_timeStamps.find( view ) );
    if ( it != m_timeStamps.end() )
      m_timeStamps.erase( it );
    m_leftBar->removeTab(view);
    m_rightBar->removeTab(view);
    m_bottomBar->removeTab(view);
}

void MainWindowIDEAl::setViewAvailable(QWidget * /*pView*/, bool /*bEnabled*/) {
    // TODO: implement me
}

void MainWindowIDEAl::raiseView(QWidget *view) {
    m_leftBar->raiseWidget(view);
    m_rightBar->raiseWidget(view);
    m_bottomBar->raiseWidget(view);
    m_tabWidget->showPage(view);
}


void MainWindowIDEAl::lowerView(QWidget *view) {
    m_leftBar->lowerWidget(view);
    m_rightBar->lowerWidget(view);
    m_bottomBar->lowerWidget(view);
}


void MainWindowIDEAl::lowerAllViews() {
    m_leftBar->lowerAllWidgets();
    m_rightBar->lowerAllWidgets();
    m_bottomBar->lowerAllWidgets();
}

void MainWindowIDEAl::moveRelativeTab(int n) {
    KTabZoomWidget * bar = 0;
    if (m_leftBar->hasFocus()) bar = m_leftBar;
    if (m_rightBar->hasFocus()) bar = m_rightBar;
    if (m_bottomBar->hasFocus()) bar = m_bottomBar;

    if (bar) {
	if(bar->count()) {
	    int index = bar->indexOf(bar->current());

	    QWidget * view = (bar->at((bar->count()+index+n)%bar->count()));
	    bar->raiseWidget(view);
	}
	return;
    }

    //Default : editor
    if(m_tabWidget->count()) {
	int index = m_tabWidget->currentPageIndex();

	QWidget * view = (m_tabWidget->page((m_tabWidget->count()+index+n)%m_tabWidget->count()));
	m_tabWidget->showPage(view);
	m_tabWidget->currentPage()->setFocus();
    }
}

void MainWindowIDEAl::gotoNextWindow() {
    moveRelativeTab(1);
}

void MainWindowIDEAl::gotoFirstWindow() {
    if (m_tabWidget->count() == 0) {
        return;
    }

    //Activates the view we accessed the most time ago
    QWidget * view = m_tabWidget->currentPage();
    QMap<QWidget*, QDateTime>::iterator it( m_timeStamps.find( view ) );
    QMap<QDateTime,QWidget*> m;

    for (it = m_timeStamps.begin(); it != m_timeStamps.end(); ++it) {
        m.insert(it.data(), it.key());
    }
    QDateTime current = m_timeStamps[view];
    QMap<QDateTime,QWidget*>::iterator pos( m.find(current) );
    QMap<QDateTime,QWidget*>::iterator newPos = pos;
    if (pos != m.end()) {
        ++newPos;
    }
    if (newPos != m.end()) { // look ahead
        ++pos;
    }
    else {
        pos = m.begin();
    }
    m_bSwitching= true; // flag that we are currently switching between windows
    m_tabWidget->showPage(pos.data());
    m_tabWidget->currentPage()->setFocus();
}

void MainWindowIDEAl::gotoPreviousWindow() {
    moveRelativeTab(-1);
}

void MainWindowIDEAl::gotoLastWindow() {
    if (m_tabWidget->count() == 0) {
        return;
    }

    //Activates the previously accessed view before this one was activated
    QWidget * view = m_tabWidget->currentPage();
    QMap<QWidget*, QDateTime>::iterator it( m_timeStamps.find( view ) );
    QMap<QDateTime,QWidget*> m;

    for ( it = m_timeStamps.begin(); it != m_timeStamps.end(); ++it) {
        m.insert(it.data(), it.key());
    }

    QDateTime current = m_timeStamps[view];
    QMap<QDateTime,QWidget*>::iterator pos( m.find(current) );
    if ( pos != m.begin() ) {
        --pos;
    }
    else {
        pos = m.end();
        --pos;
    }
    m_bSwitching= true; // flag that we are currently switching between windows
    m_tabWidget->showPage(pos.data());
    m_tabWidget->currentPage()->setFocus();
}

void MainWindowIDEAl::createGUI(KParts::Part *part) {
    if ( !part )
        setCaption( QString::null );
    KParts::MainWindow::createGUI(part);
}


void MainWindowIDEAl::loadSettings() {
    KConfig *config = kapp->config();

    ProjectManager::getInstance()->loadSettings();
    applyMainWindowSettings(config, "Mainwindow");

    config->setGroup("LeftBar");
    m_leftBar->loadSettings(config);

    config->setGroup("RightBar");
    m_rightBar->loadSettings(config);

    config->setGroup("BottomBar");
    m_bottomBar->loadSettings(config);
}

void MainWindowIDEAl::saveSettings() {
    KConfig *config = kapp->config();

    ProjectManager::getInstance()->saveSettings();
    saveMainWindowSettings(config, "Mainwindow");

    config->setGroup("LeftBar");
    m_leftBar->saveSettings(config);

    config->setGroup("RightBar");
    m_leftBar->saveSettings(config);

    config->setGroup("BottomBar");
    m_bottomBar->saveSettings(config);
}

//=============== slotFillWindowMenu ===============//
void MainWindowIDEAl::slotFillWindowMenu() {
    // construct the menu and its submenus
    bool bNoViewOpened = true;    // Assume no view is open yet
    m_pWindowMenu->clear();       // Erase whole window menu

    // Construct fixed enties of the window menu
    int closeId         = m_pWindowMenu->insertItem(i18n("&Close"), PartController::getInstance(), SLOT(slotCloseWindow()));
    int closeAllId      = m_pWindowMenu->insertItem(i18n("Close &All"), PartController::getInstance(), SLOT(slotCloseAllWindows()));
    int closeAllOtherId = m_pWindowMenu->insertItem(i18n("Close All &Others"), PartController::getInstance(), SLOT(slotCloseOtherWindows()));
    m_pWindowMenu->insertSeparator();

    int entryCount = m_pWindowMenu->count();

    QValueList<QDateTime> timeStamps;
    // Loop over all parts and add them to the window menu
    QPtrListIterator<KParts::Part> it(*(PartController::getInstance()->parts()));
    for ( ; it.current(); ++it) {
        KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
        if (!ro_part)
            continue;
        // We fond a KPart to add
      QString name = ro_part->url().prettyURL();

      unsigned int windowItemCount = m_pWindowMenu->count(); //  - entryCount;
      QString tmpString;

      // sort entries by name or last viewed time
      QDateTime timeStamp( m_timeStamps[ro_part->widget()] );
      QValueList<QDateTime>::iterator timeStampIterator = timeStamps.begin();
      unsigned int indx;
      for (indx = entryCount; indx < windowItemCount; indx++, ++timeStampIterator) {
        bool putHere = false;
        if ( (*timeStampIterator) < timeStamp )
        {
          putHere = true;
          timeStamps.insert( timeStampIterator, timeStamp );
        }
        if ( putHere )
          break;
    }
        KToggleAction *action = new KToggleAction(name, 0, 0, name.latin1());
        action->setChecked(ro_part == PartController::getInstance()->activePart());
        connect(action, SIGNAL(activated()), this, SLOT(slotBufferSelected()));
    action->plug(m_pWindowMenu, indx);
        bNoViewOpened = false;   // Now we know that at least one view exists.
    }

    if (bNoViewOpened) { // If there is no view open all fixed window menu entries will be disabled
        m_pWindowMenu->setItemEnabled(closeId, FALSE);
        m_pWindowMenu->setItemEnabled(closeAllId, FALSE);
        m_pWindowMenu->setItemEnabled(closeAllOtherId, FALSE);
    }
}

void MainWindowIDEAl::slotStatusChange(KParts::Part *p)
{
  m_timeStamps[p->widget()] = QDateTime::currentDateTime();
}

void MainWindowIDEAl::slotTabSelected(QWidget *view)
{
  if ( !switching() ) {
    m_timeStamps[view] = QDateTime::currentDateTime();
  }
}

//=============== slotBufferSelected ===============//
void MainWindowIDEAl::slotBufferSelected() {
    // Get the URL of the sender
    QString SenderName = sender()->name();
    KURL SenderUrl(SenderName);

    // Loop over all KParts
    QPtrListIterator<KParts::Part> it(*(PartController::getInstance()->parts()));
    for ( ; it.current(); ++it) {
        KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
        if (ro_part) {
            KURL PartUrl=ro_part->url();
            QString PartName=PartUrl.path();
            if (SenderUrl == PartUrl)  { // Found part to activate
                PartController::getInstance()->setActivePart(ro_part);
                if (ro_part->widget()) {
                    raiseView(ro_part->widget());
                    ro_part->widget()->setFocus();
                    m_timeStamps[ro_part->widget()] = QDateTime::currentDateTime();
                }
                break;
            }
        }
    }
}

void MainWindowIDEAl::slotPartAdded(KParts::Part* part) {

    if ( !part || !part->inherits("KTextEditor::Document") )
        return;

    connect( part, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
}

void MainWindowIDEAl::raiseTabbar( KTabZoomWidget* tabBar ) {
    if ( !tabBar )
        return;

    if ( tabBar->isRaised() ) {
        if ( tabBar->isDocked() ) {
            if ( tabBar->hasFocus() ) {
                if ( m_tabWidget->currentPage() )
                    m_tabWidget->currentPage()->setFocus();
            } else {
                tabBar->setFocus();
            }
        } else {
            tabBar->lowerAllWidgets();
            if ( m_tabWidget->currentPage() )
                m_tabWidget->currentPage()->setFocus();
        }
    } else {
        tabBar->raiseWidget( 0 );
    }
}

void MainWindowIDEAl::raiseLeftTabbar() {
    raiseTabbar( m_leftBar );
}

void MainWindowIDEAl::raiseRightTabbar() {
    raiseTabbar( m_rightBar );
}

void MainWindowIDEAl::raiseBottomTabbar() {
    raiseTabbar( m_bottomBar );
}

void MainWindowIDEAl::raiseEditor() {
    if(m_tabWidget->currentPage()) m_tabWidget->currentPage()->setFocus();
}

void MainWindowIDEAl::slotTextChanged() {
    QWidget* w = m_tabWidget->currentPage();
    if ( !w )
        return;

    QString t = m_tabWidget->tabLabel( w );
    if ( t.right( 1 ) != "*" ){
        t += "*";
	m_tabWidget->changeTab( w, t );
    }
}

void MainWindowIDEAl::slotUpdateModifiedFlags()
{
    QString newTitle;
    QPtrListIterator<KParts::Part> it(*(PartController::getInstance()->parts()));
    for ( ; it.current(); ++it) {
        KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*>(it.current());
        if ( rw_part && rw_part->widget() ) {
	    newTitle = rw_part->url().fileName();
	    if ( rw_part->isModified() )
                newTitle += "*";
            if ( m_tabWidget->tabLabel( rw_part->widget() ) != newTitle ) {
		m_tabWidget->changeTab( rw_part->widget(), newTitle );
		m_tabWidget->setTabToolTip( rw_part->widget(), rw_part->url().url() );
	    }
        }
    }
}

void MainWindowIDEAl::slotBottomTabsChanged() {
    if ( !m_bottomBar )
        return;
    m_raiseBottomBar->setEnabled( !m_bottomBar->isEmpty() );
}

void MainWindowIDEAl::slotRightTabsChanged() {
    if ( !m_rightBar )
        return;
    m_raiseRightBar->setEnabled( !m_rightBar->isEmpty() );
}

void MainWindowIDEAl::slotLeftTabsChanged() {
    if ( !m_leftBar )
        return;
    m_raiseLeftBar->setEnabled( !m_leftBar->isEmpty() );
}

void MainWindowIDEAl::slotQuit() {
    (void) queryClose();
}

bool MainWindowIDEAl::eventFilter( QObject * /*obj*/, QEvent *e )
{
  if ( e->type() == QEvent::KeyRelease )
  {
      if ( switching() )
      {
        KAction *a = actionCollection()->action( "view_next_window" ) ;
        if ( a )
        {
            const KShortcut cut( a->shortcut() );
            const KKeySequence& seq = cut.seq( 0 );
            const KKey& key = seq.key(0);
            int modFlags = key.modFlags();
            int state = ((QKeyEvent *)e)->state();
            KKey key2( (QKeyEvent *)e );

            /** these are quite some assumptions:
            *   The key combination uses exactly one modifier key
            *   The WIN button in KDE is the meta button in Qt
            **/
            if ( state != ((QKeyEvent *)e)->stateAfter()                             &&
                ((modFlags & KKey::CTRL) > 0) == ((state & Qt::ControlButton) > 0 ) &&
                ((modFlags & KKey::ALT) > 0)  == ((state & Qt::AltButton) > 0)      &&
                ((modFlags & KKey::WIN) > 0)  == ((state & Qt::MetaButton) > 0) )
            {
              if ( m_tabWidget->count() ) {
                // int index = m_tabWidget->currentPageIndex();
                QWidget * view = (m_tabWidget->currentPage());
                m_timeStamps[view] = QDateTime::currentDateTime();
              }

              // activeWindow()->updateTimeStamp();
              setSwitching( false );
            }
            return true;
        }
        else
          kdDebug(9000) <<  "KAction( \"view_next_window\") __not__ found.in MainWindowIDEAl\n" << endl;;
      }
  }
  return FALSE;  // standard event processing
}

#include "mainwindowideal.moc"

#include <qlayout.h>
#include <qmultilineedit.h>
#include <qvbox.h>
#include <qcheckbox.h>

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
#include "kdevplugin.h"


#include "projectmanager.h"
#include "plugincontroller.h"
#include "partcontroller.h"
#include "kdevpartcontroller.h"
#include "partselectwidget.h"
#include "api.h"
#include "core.h"
#include "settingswidget.h"
#include "statusbar.h"
#include "kpopupmenu.h"
#include "kmenubar.h"


#include "toplevel.h"
#include "toplevel_sdi.h"


TopLevelSDI::TopLevelSDI(QWidget *parent, const char *name)
  : KParts::MainWindow(parent, name),
  m_pWindowMenu(0L),
  m_closing(false)
{
  KAction * action;

  action = new KAction( i18n("&Next Window"), ALT+Key_PageDown, 
                        this, SLOT(gotoNextWindow()),
                        actionCollection(), "view_next_window");
  action->setStatusText( i18n("Switches to the next window") );

  action = new KAction( i18n("&Previous Window"), ALT+Key_PageUp,
                        this, SLOT(gotoPreviousWindow()),
                        actionCollection(), "view_previous_window");
  action->setStatusText( i18n("Switches to the previous window") );

   // Add window menu to the menu bar
   m_pWindowMenu = new QPopupMenu( main(), "window_menu");
   m_pWindowMenu->setCheckable( TRUE);
   menuBar()->insertItem(tr("&Window"),m_pWindowMenu);

   QObject::connect( m_pWindowMenu, SIGNAL(aboutToShow()), main(), SLOT(slotFillWindowMenu()) );

}


void TopLevelSDI::init()
{
#if (KDE_VERSION > 305)
  setStandardToolBarMenuEnabled( true );
#endif
  setXMLFile("gideonui.rc");

  createFramework();
  createActions();
  createStatusBar();

  createGUI(0);
  slotFillWindowMenu();  // Just in case there is no file open. The menu would then be empty.

  if ( PluginController::pluginServices().isEmpty() ) {
    KMessageBox::sorry( this, i18n("Unable to find plugins, KDevelop won't work properly!\nPlease make sure "
	"that KDevelop is installed in your KDE directory, otherwise you have to add KDevelop's installation "
	"path to the environment variable KDEDIRS and run kbuildsycoca. Restart KDevelop afterwards.\n"
	"Example for BASH users:\nexport KDEDIRS=/path/to/gideon:$KDEDIRS && kbuildsycoca"),
	i18n("Couldn't find plugins") );
  }
}


TopLevelSDI::~TopLevelSDI()
{
  TopLevel::invalidateInstance( this );
  delete m_pWindowMenu;
}


bool TopLevelSDI::queryClose()
{
  if (m_closing)
    return true;

  emit wantsToQuit();
  return false;
}


void TopLevelSDI::realClose()
{
  saveSettings();
  
  m_closing = true;
  close();
}


KMainWindow *TopLevelSDI::main()
{
  return this;
}


void TopLevelSDI::createStatusBar()
{
  (void) new StatusBar(this);
}


void TopLevelSDI::createFramework()
{
  m_leftBar = new KTabZoomWidget(this, KTabZoomPosition::Left);
  setCentralWidget(m_leftBar);

  m_bottomBar = new KTabZoomWidget(m_leftBar, KTabZoomPosition::Bottom);
  m_leftBar->addContent(m_bottomBar);

  m_rightBar = new KTabZoomWidget ( m_bottomBar, KTabZoomPosition::Right );
  m_bottomBar->addContent ( m_rightBar );

  m_tabWidget = new QTabWidget(m_rightBar);
  m_tabWidget->setMargin(2);

  PartController::createInstance(m_tabWidget);

  m_bottomBar->addContent(m_tabWidget);
  m_rightBar->addContent ( m_tabWidget );

  connect(m_tabWidget, SIGNAL(currentChanged(QWidget*)), 
	  PartController::getInstance(), SLOT(slotCurrentChanged(QWidget*)));

  connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)),
	  this, SLOT(createGUI(KParts::Part*)));

  connect(PartController::getInstance(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotPartAdded(KParts::Part*)));
  connect(PartController::getInstance(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotFillWindowMenu()));
  connect(PartController::getInstance(), SIGNAL(partRemoved(KParts::Part*)), this, SLOT(slotFillWindowMenu()));
  connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotFillWindowMenu()));
  connect(PartController::getInstance(), SIGNAL(savedFile(const QString&)), this, SLOT(slotUpdateModifiedFlags()));
}



void TopLevelSDI::createActions()
{
  ProjectManager::getInstance()->createActions( actionCollection() );
  
  KStdAction::quit(this, SLOT(slotQuit()), actionCollection());
    
  KAction* action;
  
  m_stopProcesses = new KAction( i18n( "&Stop" ), "stop", 
                Key_Escape, Core::getInstance(), SIGNAL(stopButtonClicked()),
                actionCollection(), "stop_processes" );
  m_stopProcesses->setStatusText(i18n("Stop all running processes"));
  m_stopProcesses->setEnabled( false );
  
  connect( Core::getInstance(), SIGNAL(activeProcessCountChanged(uint)),
           this, SLOT(slotActiveProcessCountChanged(uint)) );
 
  action = KStdAction::showMenubar(
     this, SLOT(slotShowMenuBar()),
     actionCollection(), "settings_show_menubar" );
  action->setStatusText(i18n("Lets you switch the menubar on/off"));
   
  action = KStdAction::keyBindings(
      this, SLOT(slotKeyBindings()),
      actionCollection(), "settings_configure_shortcuts" );
  action->setStatusText(i18n("Lets you configure shortcut keys"));

  action = KStdAction::configureToolbars(
      this, SLOT(slotConfigureToolbars()),
      actionCollection(), "settings_configure_toolbars" );
  action->setStatusText(i18n("Lets you configure toolbars"));

#if (KDE_VERSION > 305)
  action = KStdAction::configureNotifications(
      this, SLOT(slotConfigureNotifications()),
      actionCollection(), "settings_configure_notifications" );
  action->setStatusText(i18n("Lets you configure system notifications"));
#endif  
  
  action = KStdAction::preferences(this, SLOT(slotSettings()),
                actionCollection(), "settings_configure" );
  action->setStatusText( i18n("Lets you customize KDevelop") );
}

void TopLevelSDI::slotActiveProcessCountChanged( uint active )
{
  m_stopProcesses->setEnabled( active > 0 );
}

void TopLevelSDI::slotQuit()
{
  (void) queryClose();
}


void TopLevelSDI::embedPartView(QWidget *view, const QString &name)
{
  m_tabWidget->addTab(view, name);
  m_tabWidget->showPage(view);
}


void TopLevelSDI::embedSelectView(QWidget *view, const QString &name)
{
  m_leftBar->addTab(view, name);
}

void TopLevelSDI::embedSelectViewRight ( QWidget* view, const QString& title )
{
	m_rightBar->addTab ( view, title );
}

void TopLevelSDI::embedOutputView(QWidget *view, const QString &name)
{
  m_bottomBar->addTab(view, name);
}


void TopLevelSDI::removeView(QWidget *)
{
}

void TopLevelSDI::setViewVisible(QWidget *pView, bool bEnabled)
{
  // TODO: implement me
}

void TopLevelSDI::raiseView(QWidget *view)
{
  m_leftBar->raiseWidget(view);
  m_bottomBar->raiseWidget(view);
  m_tabWidget->showPage(view);
}


void TopLevelSDI::lowerView(QWidget *view)
{
  m_leftBar->lowerWidget(view);
  m_bottomBar->lowerWidget(view);
}


void TopLevelSDI::lowerAllViews()
{
  m_leftBar->lowerAllWidgets();
  m_bottomBar->lowerAllWidgets();
}

void TopLevelSDI::moveRelativeTab(unsigned int n)
{
  if(m_tabWidget->count()) {
    int index = m_tabWidget->currentPageIndex();
		
    QWidget * view = (m_tabWidget->page((index+n)%m_tabWidget->count()));
    m_tabWidget->showPage(view);
  }
}

void TopLevelSDI::gotoNextWindow()
{
  moveRelativeTab(1);
}

void TopLevelSDI::gotoPreviousWindow()
{
  moveRelativeTab(m_tabWidget->count()-1);
}

void TopLevelSDI::createGUI(KParts::Part *part)
{
  KParts::MainWindow::createGUI(part);
}


void TopLevelSDI::loadSettings()
{
  KConfig *config = kapp->config();
  
  ProjectManager::getInstance()->loadSettings();
  applyMainWindowSettings(config, "Mainwindow");

  config->setGroup("LeftBar");
  m_leftBar->loadSettings(config);

  config->setGroup("BottomBar");
  m_bottomBar->loadSettings(config);
}


void TopLevelSDI::saveSettings()
{
  KConfig *config = kapp->config();

  ProjectManager::getInstance()->saveSettings();
  saveMainWindowSettings(config, "Mainwindow");

  config->setGroup("LeftBar");
  m_leftBar->saveSettings(config);

  config->setGroup("BottomBar");
  m_bottomBar->saveSettings(config);
}


void TopLevelSDI::slotKeyBindings()
{
  KKeyDialog dlg( false, this );
  QPtrList<KXMLGUIClient> clients = guiFactory()->clients();
  for( QPtrListIterator<KXMLGUIClient> it( clients );
       it.current(); ++it ) {
    dlg.insert( (*it)->actionCollection() );
  }
  dlg.configure();
}

void TopLevelSDI::slotConfigureToolbars()
{
  saveMainWindowSettings( KGlobal::config(), "Mainwindow" );
  KEditToolbar dlg( factory() );
  connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
  dlg.exec();
}

// called when OK ar Apply is clicked in the EditToolbar Dialog
void TopLevelSDI::slotNewToolbarConfig()
{
  // replug actionlists here...

  applyMainWindowSettings( KGlobal::config(), "Mainwindow" );
}

void TopLevelSDI::slotShowMenuBar()
{
  if (menuBar()->isVisible()) {
    menuBar()->hide();
  } else {
    menuBar()->show();
  }
  saveMainWindowSettings( KGlobal::config(), "Mainwindow" );
}

void TopLevelSDI::slotConfigureNotifications()
{
#if (KDE_VERSION > 305)
  KNotifyDialog::configure(this, "Notification Configuration Dialog");
#endif
}

void TopLevelSDI::slotSettings()
{
  KDialogBase dlg(KDialogBase::TreeList, i18n("Customize KDevelop"),
                  KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, this,
                  "customization dialog");

  QVBox *vbox = dlg.addVBoxPage(i18n("General"));
  SettingsWidget *gsw = new SettingsWidget(vbox, "general settings widget");
  
  KConfig* config = kapp->config();
  config->setGroup("General Options");
  gsw->lastProjectCheckbox->setChecked(config->readBoolEntry("Read Last Project On Startup",true));

  Core::getInstance()->doEmitConfigWidget(&dlg);
  dlg.exec();

  config->setGroup("General Options");
  config->writeEntry("Read Last Project On Startup",gsw->lastProjectCheckbox->isChecked());
}


//=============== slotFillWindowMenu ===============//
void TopLevelSDI::slotFillWindowMenu()
{
  // construct the menu and its submenus
  bool bNoViewOpened = true;    // Assume no view is open yet
  m_pWindowMenu->clear();       // Erase whole window menu

  // Construct fixed enties of the window menu
  int closeId         = m_pWindowMenu->insertItem(tr("&Close"), PartController::getInstance(), SLOT(slotCloseWindow()));
  int closeAllId      = m_pWindowMenu->insertItem(tr("Close &All"), PartController::getInstance(), SLOT(slotCloseAllWindows()));
  int closeAllOtherId = m_pWindowMenu->insertItem(tr("Close All &Others"), PartController::getInstance(), SLOT(slotCloseOtherWindows()));
  m_pWindowMenu->insertSeparator();

  // Loop over all parts and add them to the window menu
  QPtrListIterator<KParts::Part> it(*(PartController::getInstance()->parts()));
  for ( ; it.current(); ++it)
  {
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
    if (!ro_part)
      continue;
    // We fond a KPart to add
    QString name = ro_part->url().url();
    KToggleAction *action = new KToggleAction(name, 0, 0, name.latin1());
    action->setChecked(ro_part == PartController::getInstance()->activePart());
    connect(action, SIGNAL(activated()), this, SLOT(slotBufferSelected()));
    action->plug(m_pWindowMenu);
    bNoViewOpened = false;   // Now we know that at least one view exists.
   }

   if (bNoViewOpened) { // If there is no view open all fixed window menu entries will be disabled
      m_pWindowMenu->setItemEnabled(closeId, FALSE);
      m_pWindowMenu->setItemEnabled(closeAllId, FALSE);
      m_pWindowMenu->setItemEnabled(closeAllOtherId, FALSE);
   }
}

//=============== slotBufferSelected ===============//
void TopLevelSDI::slotBufferSelected()
{

  // Get the URL of the sender
  QString SenderName = sender()->name();
  KURL SenderUrl(SenderName);

  // Loop over all KParts
  QPtrListIterator<KParts::Part> it(*(PartController::getInstance()->parts()));
  for ( ; it.current(); ++it)
  {
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
    if (ro_part) {
      KURL PartUrl=ro_part->url();
      QString PartName=PartUrl.path();
      if (SenderUrl == PartUrl)  { // Found part to activate
        PartController::getInstance()->setActivePart(ro_part);
        if (ro_part->widget()) {
          raiseView(ro_part->widget());
          ro_part->widget()->setFocus();
        }
        break;
      }
    }
  }
  slotFillWindowMenu();  // To check the correct entry
}

void TopLevelSDI::slotPartAdded(KParts::Part* part)
{
  if ( !part || !part->inherits("KTextEditor::Document") )
    return;

  connect( part, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
}

void TopLevelSDI::slotTextChanged()
{
  QWidget* w = m_tabWidget->currentPage();
  if ( !w )
    return;

  QString t = m_tabWidget->tabLabel( w );
  if ( t.right( 1 ) != "*" )
    t += "*";

  m_tabWidget->setTabLabel( w, t ); 
}

void TopLevelSDI::slotUpdateModifiedFlags()
{
  QPtrListIterator<KParts::Part> it(*(PartController::getInstance()->parts()));
  for ( ; it.current(); ++it) {
    KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*>(it.current());
    if ( rw_part && rw_part->widget() ) {
      int idx = m_tabWidget->indexOf( rw_part->widget() );
      if ( idx < 0 )
        continue;
      QString t = m_tabWidget->tabLabel( rw_part->widget() );
      bool titleMod = (t.right( 1 ) == "*");
      if ( rw_part->isModified() && !titleMod ) {
	m_tabWidget->setTabLabel( rw_part->widget(), t + "*" );
      } else if ( !rw_part->isModified() && titleMod ) {
        t.truncate( t.length() - 1 );
        m_tabWidget->setTabLabel( rw_part->widget(), t );
      }
    }
  }
}

#include "toplevel_sdi.moc"

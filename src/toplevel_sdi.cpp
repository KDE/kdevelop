#include <qlayout.h>
#include <qmultilineedit.h>
#include <qvbox.h>
#include <qcheckbox.h>


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
#include <kedittoolbar.h>


#include "widgets/ktabzoomwidget.h"
#include "kdevplugin.h"


#include "projectmanager.h"
#include "partcontroller.h"
#include "partselectwidget.h"
#include "api.h"
#include "core.h"
#include "settingswidget.h"
#include "statusbar.h"

#include "toplevel.h"
#include "toplevel_sdi.h"


TopLevelSDI::TopLevelSDI(QWidget *parent, const char *name)
  : KParts::MainWindow(parent, name), m_closing(false)
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
}


void TopLevelSDI::init()
{
  setStandardToolBarMenuEnabled( true );
  setXMLFile("gideonui.rc");

  createFramework();
  createActions();
  createStatusBar();

  createGUI(0);
}


TopLevelSDI::~TopLevelSDI()
{
  TopLevel::invalidateInstance( this );
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


  vbox = dlg.addVBoxPage(i18n("Plugins"));
  PartSelectWidget *w = new PartSelectWidget(vbox, "part selection widget");
  connect( &dlg, SIGNAL(okClicked()), w, SLOT(accept()) );

  Core::getInstance()->doEmitConfigWidget(&dlg);
  dlg.exec();

  config->setGroup("General Options");
  config->writeEntry("Read Last Project On Startup",gsw->lastProjectCheckbox->isChecked());
}



#include "toplevel_sdi.moc"

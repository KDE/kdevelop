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
#include <kconfig.h>
#include <kstatusbar.h>
#include <kdialogbase.h>
#include <kkeydialog.h>
#include <kedittoolbar.h>
#include <kmenubar.h>
#include <kmessagebox.h>


#include "projectmanager.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "api.h"
#include "core.h"
#include "settingswidget.h"
#include "statusbar.h"

#include "toplevel.h"
#include "toplevel_mdi.h"


TopLevelMDI::TopLevelMDI(QWidget *parent, const char *name)
  : QextMdiMainFrm(parent, name), m_closing(false)
{
	KAction * action;

	action = new KAction( i18n("&Next Window"), ALT+Key_PageDown, this, SLOT(gotoNextWindow()),actionCollection(), "view_next_window");
  action->setStatusText( i18n("Switches to the next window") );

	action = new KAction( i18n("&Previous Window"), ALT+Key_PageUp, this, SLOT(gotoPreviousWindow()),actionCollection(), "view_previous_window");
  action->setStatusText( i18n("Switches to the previous window") );

}


void TopLevelMDI::init()
{
#if (KDE_VERSION > 305)
  setStandardToolBarMenuEnabled( true );
#endif
  setXMLFile("gideonui.rc");

  createFramework();
  createActions();
  createStatusBar();

  createGUI(0);

  if ( PluginController::pluginServices().isEmpty() ) {
    KMessageBox::sorry( this, i18n("Unable to find plugins, KDevelop won't work properly!\nPlease make sure "
        "that KDevelop is installed in your KDE directory, otherwise you have to add KDevelop's installation "
        "path to the environment variable KDEDIRS and run kbuildsycoca. Restart KDevelop afterwards.\n"
        "Example for BASH users:\nexport KDEDIRS=/path/to/gideon:$KDEDIRS && kbuildsycoca"),
        i18n("Couldn't find plugins") );
  }
}


TopLevelMDI::~TopLevelMDI()
{
  TopLevel::invalidateInstance( this );
}


bool TopLevelMDI::queryClose()
{
  if (m_closing)
    return true;

  emit wantsToQuit();
  return false;
}


void TopLevelMDI::realClose()
{
  saveSettings();

  m_closing = true;
  close();
}


KMainWindow *TopLevelMDI::main()
{
  return this;
}


void TopLevelMDI::createStatusBar()
{
  (void) new StatusBar(this);
}


void TopLevelMDI::createFramework()
{
  if (!isFakingSDIApplication())
  {
    menuBar()->insertItem( tr("&Window"), windowMenu());
  }

  PartController::createInstance(this);

  connect(PartController::getInstance(), SIGNAL(activePartChanged(KParts::Part*)),
	  this, SLOT(createGUI(KParts::Part*)));

  setMenuForSDIModeSysButtons(menuBar());
}


void TopLevelMDI::createActions()
{
  ProjectManager::getInstance()->createActions( actionCollection() );
  
  KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

  KAction *action;
  
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
  action->setStatusText(i18n("Lets you customize KDevelop") );
}

void TopLevelMDI::slotActiveProcessCountChanged( uint active )
{
  m_stopProcesses->setEnabled( active > 0 );
}

void TopLevelMDI::slotQuit()
{
  (void) queryClose();
}


QextMdiChildView *TopLevelMDI::wrapper(QWidget *view, const QString &name)
{
  Q_ASSERT( view ); // if this assert fails, then some part didn't return a widget. Fix the part ;)

  QextMdiChildView* pMDICover = new QextMdiChildView(name);
  QBoxLayout* pLayout = new QHBoxLayout( pMDICover, 0, -1, "layout");
  view->reparent(pMDICover, QPoint(0,0));
  pLayout->addWidget(view);
  pMDICover->setName(name);
  QString shortName = name;
  int length = shortName.length();
  shortName = shortName.right(length - (shortName.findRev('/') +1));
  pMDICover->setTabCaption(shortName);
  pMDICover->setCaption(name);

  m_widgetMap.insert(view, pMDICover);
  m_childViewMap.insert(pMDICover, view);

  return pMDICover;
}


void TopLevelMDI::embedPartView(QWidget *view, const QString &name)
{
  // Bugfix for addWindow() using the wrong parent when a toolview is activated...
  // Don't activate one unless necessary & possible
  if (m_partViews.find(activeWindow()) == -1 && m_partViews.first()) {
    activateView(m_partViews.first());
  }

  QextMdiChildView *child = wrapper(view, name);

  unsigned int mdiFlags = QextMdi::StandardAdd | QextMdi::Maximize;
  
  addWindow(child, QPoint(0,0), mdiFlags);

  m_partViews.append(child);
}


void TopLevelMDI::embedSelectView(QWidget *view, const QString &name)
{
  QWidget *first = m_selectViews.first();

  QextMdiChildView *child = wrapper(view, name);

  if (!first)  // If there is no selected view yet ...
  {
    if (mdiMode() == QextMdi::TabPageMode)
      first = m_partViews.first();

    if (!first)
      first = this;
    
    addToolWindow(child, KDockWidget::DockLeft, first, 25, name, name);
  }
  else
    addToolWindow(child, KDockWidget::DockCenter, first, 25, name, name);

  m_selectViews.append(child);
}

void TopLevelMDI::embedSelectViewRight ( QWidget* view, const QString& title )
{
  //we do not have a right pane so we insert it in the default pos
  embedSelectView( view, title );
}

void TopLevelMDI::embedOutputView(QWidget *view, const QString &name)
{
  QWidget *first = m_outputViews.first();

  QextMdiChildView *child = wrapper(view, name);

  if (!first)   // If there is no output view yet ...
  {
    if (mdiMode() == QextMdi::TabPageMode)
      first = m_partViews.first();

    if (!first)
      first = this;

    addToolWindow(child, KDockWidget::DockBottom, first, 70, name, name);
  }
  else
    addToolWindow(child, KDockWidget::DockCenter, first, 70, name, name);

  m_outputViews.append(child);
}


void TopLevelMDI::removeView(QWidget *view)
{
  if (!view)
    return;

  QextMdiChildView *wrapper = m_widgetMap[view];

  if (wrapper)
  {
    removeWindowFromMdi(wrapper);

    m_selectViews.remove(wrapper);
    m_outputViews.remove(wrapper);
    m_partViews.remove(wrapper);

    m_widgetMap.remove(view);
    m_childViewMap.remove(wrapper);

    // Find the KDockWidget which covers the QextMdiChildView to remove.
    // Undock the KDockWidget if there is one.
    // This will remove the corresponding tab from the output and tree views.
    KDockWidget* pDock = dockManager->findWidgetParentDock(wrapper);
    if (pDock)
    {
      pDock->undock();
    }

    // Note: this reparenting is necessary. Otherwise, the view gets
    // deleted twice: once when the wrapper is deleted, and the second
    // time when the part is deleted.
    view->reparent(this, QPoint(0,0), false);

    closeWindow(wrapper);
  }
}


void TopLevelMDI::raiseView(QWidget *view)
{
  QextMdiChildView *wrapper = m_widgetMap[view];
  if (wrapper)
  {
    wrapper->activate();
    activateView(wrapper);
  }
}


void TopLevelMDI::lowerView(QWidget *)
{
  // ignored in MDI mode!
}

void TopLevelMDI::lowerAllViews()
{
    // ignored in MDI mode!
}


void TopLevelMDI::createGUI(KParts::Part *part)
{
  QextMdiMainFrm::createGUI(part);
}


void TopLevelMDI::loadSettings()
{
  ProjectManager::getInstance()->loadSettings();
  loadMDISettings();
  applyMainWindowSettings(kapp->config(), "Mainwindow");
}


void TopLevelMDI::loadMDISettings()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  int mdiMode = config->readNumEntry("MDI mode", QextMdi::ChildframeMode);
  switch (mdiMode) 
  {
  case QextMdi::ToplevelMode:
    {
      int childFrmModeHt = config->readNumEntry("Childframe mode height", kapp->desktop()->height() - 50);
      resize(width(), childFrmModeHt);
      switchToToplevelMode();
    }
    break;
  
  case QextMdi::ChildframeMode:
    break;
  
  case QextMdi::TabPageMode:
    {
      int childFrmModeHt = config->readNumEntry("Childframe mode height", kapp->desktop()->height() - 50);
      resize(width(), childFrmModeHt);
      switchToTabPageMode();
    }
    break;
  
  default:
    break;
  }
  
  // restore a possible maximized Childframe mode
  bool maxChildFrmMode = config->readBoolEntry("maximized childframes", true);
  setEnableMaximizedChildFrmMode(maxChildFrmMode);
}


void TopLevelMDI::saveSettings()
{
  ProjectManager::getInstance()->saveSettings();
  saveMainWindowSettings(kapp->config(), "Mainwindow");
}


void TopLevelMDI::saveMDISettings()
{
  KConfig *config = kapp->config();
  config->setGroup("UI");

  config->writeEntry("MDI mode", mdiMode());
}


void TopLevelMDI::slotKeyBindings()
{
  KKeyDialog dlg( false, this );
  QPtrList<KXMLGUIClient> clients = guiFactory()->clients();
  for( QPtrListIterator<KXMLGUIClient> it( clients );
       it.current(); ++it ) {
    dlg.insert( (*it)->actionCollection() );
  }
  dlg.configure();
}

void TopLevelMDI::slotConfigureToolbars()
{
  saveMainWindowSettings( KGlobal::config(), "Mainwindow" );
  KEditToolbar dlg( factory() );
  connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
  dlg.exec();
}

// called when OK ar Apply is clicked in the EditToolbar Dialog
void TopLevelMDI::slotNewToolbarConfig()
{
  // replug actionlists here...

  applyMainWindowSettings( KGlobal::config(), "Mainwindow" );
}

void TopLevelMDI::slotShowMenuBar()
{
  if (menuBar()->isVisible()) {
    menuBar()->hide();
  } else {
    menuBar()->show();
  }
  saveMainWindowSettings( KGlobal::config(), "Mainwindow" );
}

void TopLevelMDI::slotSettings()
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


void TopLevelMDI::resizeEvent(QResizeEvent *ev)
{
  QextMdiMainFrm::resizeEvent(ev);
  setSysButtonsAtMenuPosition();
}

void TopLevelMDI::childWindowCloseRequest(QextMdiChildView *pWnd)
{
  PartController::getInstance()->closePartForWidget( m_childViewMap[pWnd] );
}

void TopLevelMDI::gotoNextWindow()
{
  activateNextWin();
}

void TopLevelMDI::gotoPreviousWindow()
{
  activatePrevWin();
}

//=============== fillWindowMenu ===============//
void TopLevelMDI::fillWindowMenu()
{
   bool bTabPageMode = FALSE;
   if (m_mdiMode == QextMdi::TabPageMode)
      bTabPageMode = TRUE;

   bool bNoViewOpened = FALSE;
   if (m_pWinList->isEmpty()) {
      bNoViewOpened = TRUE;
   }
   // construct the menu and its submenus
   if (!m_bClearingOfWindowMenuBlocked) {
      m_pWindowMenu->clear();
   }
   int closeId         = m_pWindowMenu->insertItem(tr("&Close"), PartController::getInstance(), SLOT(slotCloseWindow()));
   int closeAllId      = m_pWindowMenu->insertItem(tr("Close &All"), PartController::getInstance(), SLOT(slotCloseAllWindows()));
   int closeAllOtherId = m_pWindowMenu->insertItem(tr("Close All &Others"), PartController::getInstance(), SLOT(slotCloseOtherWindows()));
   if (bNoViewOpened) {
      m_pWindowMenu->setItemEnabled(closeId, FALSE);
      m_pWindowMenu->setItemEnabled(closeAllId, FALSE);
      m_pWindowMenu->setItemEnabled(closeAllOtherId, FALSE);
   }
   if (!bTabPageMode) {
      int iconifyId = m_pWindowMenu->insertItem(tr("&Minimize All"), this, SLOT(iconifyAllViews()));
      if (bNoViewOpened) {
         m_pWindowMenu->setItemEnabled(iconifyId, FALSE);
      }
   }
   m_pWindowMenu->insertSeparator();
   m_pWindowMenu->insertItem(tr("&MDI Mode..."), m_pMdiModeMenu);
   m_pMdiModeMenu->clear();
   m_pMdiModeMenu->insertItem(tr("&Toplevel mode"), this, SLOT(switchToToplevelMode()));
   m_pMdiModeMenu->insertItem(tr("C&hildframe mode"), this, SLOT(switchToChildframeMode()));
   m_pMdiModeMenu->insertItem(tr("Ta&b Page mode"), this, SLOT(switchToTabPageMode()));
   switch (m_mdiMode) {
   case QextMdi::ToplevelMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(0), TRUE);
      break;
   case QextMdi::ChildframeMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(1), TRUE);
      break;
   case QextMdi::TabPageMode:
      m_pMdiModeMenu->setItemChecked(m_pMdiModeMenu->idAt(2), TRUE);
      break;
   default:
      break;
   }
   m_pWindowMenu->insertSeparator();
   if (!bTabPageMode) {
      int placMenuId = m_pWindowMenu->insertItem(tr("&Tile..."), m_pPlacingMenu);
      m_pPlacingMenu->clear();
      m_pPlacingMenu->insertItem(tr("Ca&scade windows"), m_pMdi,SLOT(cascadeWindows()));
      m_pPlacingMenu->insertItem(tr("Cascade &maximized"), m_pMdi,SLOT(cascadeMaximized()));
      m_pPlacingMenu->insertItem(tr("Expand &vertically"), m_pMdi,SLOT(expandVertical()));
      m_pPlacingMenu->insertItem(tr("Expand &horizontally"), m_pMdi,SLOT(expandHorizontal()));
      m_pPlacingMenu->insertItem(tr("Tile &non-overlapped"), m_pMdi,SLOT(tileAnodine()));
      m_pPlacingMenu->insertItem(tr("Tile overla&pped"), m_pMdi,SLOT(tilePragma()));
      m_pPlacingMenu->insertItem(tr("Tile v&ertically"), m_pMdi,SLOT(tileVertically()));
      if (m_mdiMode == QextMdi::ToplevelMode) {
         m_pWindowMenu->setItemEnabled(placMenuId, FALSE);
      }
      m_pWindowMenu->insertSeparator();
      int dockUndockId = m_pWindowMenu->insertItem(tr("&Dock/Undock..."), m_pDockMenu);
         m_pDockMenu->clear();
      m_pWindowMenu->insertSeparator();
      if (bNoViewOpened) {
         m_pWindowMenu->setItemEnabled(placMenuId, FALSE);
         m_pWindowMenu->setItemEnabled(dockUndockId, FALSE);
      }
   }
   int entryCount = m_pWindowMenu->count();

   // for all child frame windows: give an ID to every window and connect them in the end with windowMenuItemActivated()
   int i=100;
   QextMdiChildView* pView = 0L;
   QPtrListIterator<QextMdiChildView> it(*m_pWinList);
   for( ; it.current(); ++it) {

      pView = it.current();
      if( pView->isToolView())
         continue;

      KParts::ReadOnlyPart * ro_part = getPartFromWidget(m_childViewMap[pView]);

      QString name = (ro_part==0L)?pView->caption():ro_part->url().url();
      QString item;
      // set titles of minimized windows in brackets
      if( pView->isMinimized()) {
         item += "(";
         item += name;
         item += ")";
      }
      else {
         item += " ";
         item += name;
       }

      // insert the window entry sorted in alphabetical order
      unsigned int indx;
      unsigned int windowItemCount = m_pWindowMenu->count() - entryCount;
      bool inserted = FALSE;
      QString tmpString;
      for (indx = 0; indx <= windowItemCount; indx++) {
         tmpString = m_pWindowMenu->text( m_pWindowMenu->idAt( indx+entryCount));
         if (tmpString.right( tmpString.length()-2) > item.right( item.length()-2)) {
            m_pWindowMenu->insertItem( item, pView, SLOT(slot_clickedInWindowMenu()), 0, -1, indx+entryCount);
            if (pView == m_pCurrentWindow)
               m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt( indx+entryCount), TRUE);
            pView->setWindowMenuID( i);
            if (!bTabPageMode) {
               m_pDockMenu->insertItem( item, pView, SLOT(slot_clickedInDockMenu()), 0, -1, indx);
               if (pView->isAttached())
                  m_pDockMenu->setItemChecked( m_pDockMenu->idAt( indx), TRUE);
            }
            inserted = TRUE;
            indx = windowItemCount+1;  // break the loop
         }
      }
      if (!inserted) {  // append it
         m_pWindowMenu->insertItem( item, pView, SLOT(slot_clickedInWindowMenu()), 0, -1, windowItemCount+entryCount);
         if (pView == m_pCurrentWindow)
            m_pWindowMenu->setItemChecked( m_pWindowMenu->idAt(windowItemCount+entryCount), TRUE);
         pView->setWindowMenuID( i);
         if (!bTabPageMode) {
            m_pDockMenu->insertItem( item, pView, SLOT(slot_clickedInDockMenu()), 0, -1, windowItemCount);
            if (pView->isAttached())
               m_pDockMenu->setItemChecked( m_pDockMenu->idAt(windowItemCount), TRUE);
         }
      }
      i++;
   }
}

KParts::ReadOnlyPart * TopLevelMDI::getPartFromWidget(const QWidget * pWidget) const
{
  // Loop over all parts to search for a matching widget
  QPtrListIterator<KParts::Part> it(*(PartController::getInstance()->parts()));
  for ( ; it.current(); ++it)
  {
    KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
    if (ro_part->widget() == pWidget) return ro_part;
  }
  return (0L);
}

void TopLevelMDI::switchToToplevelMode(void)
{
  QextMdiMainFrm::switchToToplevelMode();
  saveMDISettings();
}

void TopLevelMDI::switchToChildframeMode(void)
{
  QextMdiMainFrm::switchToChildframeMode();
  saveMDISettings();
}
void TopLevelMDI::switchToTabPageMode(void)
{
  QextMdiMainFrm::switchToTabPageMode();
  saveMDISettings();
}
#include "toplevel_mdi.moc"

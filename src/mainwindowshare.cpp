/***************************************************************************
          mainwindowshare.cpp  -  shared stuff of the main widgets
                             -------------------
    begin                : 19 Dec 2002
    copyright            : (C) 2002 by Falk Brettschneider
    email                : falk@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qcheckbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qdir.h>
#include <qregexp.h>

#include <kaction.h>
#include <kaboutdata.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kparts/mainwindow.h>
#include <kdialogbase.h>
#include <kconfig.h>
#include <kkeydialog.h>
#include <kmenubar.h>
#include <kedittoolbar.h>
#include <kbugreport.h>
#include <kurlrequester.h>
#include <kpopupmenu.h>
#include <kiconloader.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/configinterface.h>
#include <kparts/partmanager.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <knotifydialog.h>


#include <config.h>

#include "partcontroller.h"
#include "projectmanager.h"
#include "core.h"
#include "api.h"
#include "kdevmakefrontend.h"
#include "toplevel.h"
#include "kdevplugincontroller.h"

#include "kdevplugininfo.h"

#include "mainwindowshare.h"

#include "shellextension.h"

#ifdef KDE_MAKE_VERSION
# if KDE_VERSION < KDE_MAKE_VERSION(3,1,90)
#  define NEED_CONFIGHACK
# endif
#else
# define NEED_CONFIGHACK
#endif

using namespace MainWindowUtils;

MainWindowShare::MainWindowShare(QObject* pParent, const char* name)
  :QObject(pParent, name)
  ,m_toggleMainToolbar(0L)
  ,m_toggleBuildToolbar(0L)
  ,m_toggleViewToolbar(0L)
  ,m_toggleBrowserToolbar(0L)
  ,m_toggleStatusbar(0L)
  ,m_stopProcesses(0L)
{
  m_pMainWnd = (KParts::MainWindow*)pParent;
}

void MainWindowShare::init()
{
  connect(Core::getInstance(), SIGNAL(contextMenu(QPopupMenu *, const Context *)),
          this, SLOT(contextMenu(QPopupMenu *, const Context *)));

  connect( m_pMainWnd->actionCollection(), SIGNAL( actionStatusText( const QString & ) ),
        this, SLOT( slotActionStatusText( const QString & ) ) );
}

void MainWindowShare::slotActionStatusText( const QString &text )
{
//    kdDebug(9000) << "MainWindowShare::slotActionStatusText() - " << text << endl;

    if ( ! m_pMainWnd ) return;

    KStatusBar * statusBar = m_pMainWnd->statusBar();

    if ( ! statusBar ) return;

    statusBar->message( text );
}

void MainWindowShare::createActions()
{
  ProjectManager::getInstance()->createActions( m_pMainWnd->actionCollection() );

  KStdAction::quit(this->parent(), SLOT(close()), m_pMainWnd->actionCollection());

  KAction* action;

  m_stopProcesses = new KToolBarPopupAction( i18n( "&Stop" ), "stop",
                                 Key_Escape, this, SLOT(slotStopButtonPressed()),
                                 m_pMainWnd->actionCollection(), "stop_processes" );
  m_stopProcesses->setToolTip(i18n("Stop"));
  m_stopProcesses->setWhatsThis(i18n("<b>Stop</b><p>Stops all running processes (like building process, grep command, etc.). When placed onto a toolbar provides a popup menu to choose a process to stop."));
  m_stopProcesses->setEnabled( false );
  connect(m_stopProcesses->popupMenu(), SIGNAL(aboutToShow()),
         this, SLOT(slotStopMenuAboutToShow()));
  connect(m_stopProcesses->popupMenu(), SIGNAL(activated(int)),
         this, SLOT(slotStopPopupActivated(int)));

  connect( Core::getInstance(), SIGNAL(activeProcessChanged(KDevPlugin*, bool)),
           this, SLOT(slotActiveProcessChanged(KDevPlugin*, bool)) );

  action = KStdAction::showMenubar(
                this, SLOT(slotShowMenuBar()),
                m_pMainWnd->actionCollection(), "settings_show_menubar" );
  action->setToolTip(beautifyToolTip(action->text()));
  action->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(action->text())).arg(i18n("Lets you switch the menubar on/off.")));

  action = KStdAction::keyBindings(
                this, SLOT(slotKeyBindings()),
                m_pMainWnd->actionCollection(), "settings_configure_shortcuts" );
  action->setToolTip(beautifyToolTip(action->text()));
  action->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(action->text())).arg(i18n("Lets you configure shortcut keys.")));

  action = KStdAction::configureToolbars(
                this, SLOT(slotConfigureToolbars()),
                m_pMainWnd->actionCollection(), "settings_configure_toolbars" );
  action->setToolTip(beautifyToolTip(action->text()));
  action->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(action->text())).arg(i18n("Lets you configure toolbars.")));

  action = KStdAction::configureNotifications(
                this, SLOT(slotConfigureNotifications()),
                m_pMainWnd->actionCollection(), "settings_configure_notifications" );
  action->setToolTip(beautifyToolTip(action->text()));
  action->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(action->text())).arg(i18n("Lets you configure system notifications.")));

  action = KStdAction::preferences(this, SLOT(slotSettings()),
                m_pMainWnd->actionCollection(), "settings_configure" );
  action->setToolTip(beautifyToolTip(action->text()));
  action->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(action->text())).arg(i18n("Lets you customize KDevelop.")));

  m_toggleStatusbar = KStdAction::showToolbar(this, SLOT(slotToggleStatusbar()),m_pMainWnd->actionCollection(), "settings_statusbar");
  m_toggleStatusbar->setText(i18n("Show &Statusbar"));
  m_toggleStatusbar->setToolTip( i18n("Show statusbar") );
  m_toggleStatusbar->setWhatsThis(i18n("<b>Show statusbar</b><p>Hides or shows the statusbar."));

  action = new KAction( i18n("&Next Window"), ALT+Key_Right, this, SIGNAL(gotoNextWindow()),m_pMainWnd->actionCollection(), "view_next_window");
  action->setToolTip( i18n("Next window") );
  action->setWhatsThis(i18n("<b>Next window</b><p>Switches to the next window."));

  action = new KAction( i18n("&Previous Window"), ALT+Key_Left, this, SIGNAL(gotoPreviousWindow()),m_pMainWnd->actionCollection(), "view_previous_window");
  action->setToolTip( i18n("Previous window") );
  action->setWhatsThis(i18n("<b>Previous window</b><p>Switches to the previous window."));

  action = new KAction( i18n("&Last Accessed Window"), ALT+Key_Up, this, SIGNAL(gotoLastWindow()), m_pMainWnd->actionCollection(), "view_last_window");
  action->setToolTip( i18n("Last accessed window") );
  action->setWhatsThis(i18n("<b>Last accessed window</b><p>Switches to the last viewed window (Hold the Alt key pressed and walk on by repeating the Up key)."));

  action = new KAction( i18n("&First Accessed Window"), ALT+Key_Down, this, SIGNAL(gotoFirstWindow()), m_pMainWnd->actionCollection(), "view_first_window");
  action->setToolTip( i18n("First accessed window") );
  action->setWhatsThis(i18n("<b>First accessed window</b><p>Switches to the first accessed window (Hold the Alt key pressed and walk on by repeating the Down key)."));

  m_configureEditorAction = new KAction( i18n("Configure &Editor..."), 0, this, SLOT( slotConfigureEditors() ), m_pMainWnd->actionCollection(), "settings_configure_editors");
  m_configureEditorAction->setToolTip( i18n("Configure editor settings") );
  m_configureEditorAction->setWhatsThis(i18n("<b>Configure editor</b><p>Opens editor configuration dialog."));
  m_configureEditorAction->setEnabled( false );

  KDevPartController * partController = API::getInstance()->partController();
  connect( partController, SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotActivePartChanged(KParts::Part* )) );
}

void MainWindowShare::slotReportBug()
{
  KBugReport a(m_pMainWnd, true, KGlobal::instance()->aboutData() );
  a.exec();
}

void MainWindowShare::slotToggleMainToolbar()
{
  if (m_toggleMainToolbar->isChecked())
    m_pMainWnd->toolBar("mainToolBar")->show();
  else
    m_pMainWnd->toolBar("mainToolBar")->hide();
}

void MainWindowShare::slotToggleBuildToolbar()
{
  if (m_toggleBuildToolbar->isChecked())
    m_pMainWnd->toolBar("buildToolBar")->show();
  else
    m_pMainWnd->toolBar("buildToolBar")->hide();
}

void MainWindowShare::slotToggleViewToolbar()
{
  if (m_toggleViewToolbar->isChecked())
    m_pMainWnd->toolBar("viewToolBar")->show();
  else
    m_pMainWnd->toolBar("viewToolBar")->hide();
}

void MainWindowShare::slotToggleBrowserToolbar()
{
  if (m_toggleBrowserToolbar->isChecked())
    m_pMainWnd->toolBar("browserToolBar")->show();
  else
    m_pMainWnd->toolBar("browserToolBar")->hide();
}

void MainWindowShare::slotToggleStatusbar()
{
  KStatusBar* sb = (KStatusBar*) m_pMainWnd->statusBar();
  if (m_toggleStatusbar->isChecked())
    sb->show();
  else
    sb->hide();
}

void MainWindowShare::slotStopButtonPressed()
{
  Core::getInstance()->doEmitStopButtonPressed();
}

void MainWindowShare::slotActiveProcessChanged( KDevPlugin* plugin, bool active )
{
  if ( !plugin )
    return;

  if ( active ) {
    activeProcesses.append( plugin );
  } else {
    activeProcesses.removeRef( plugin );
  }
  m_stopProcesses->setEnabled( !activeProcesses.isEmpty() );
}

void MainWindowShare::slotStopPopupActivated( int id )
{
  KDevPlugin* plugin = activeProcesses.at( id );
  if ( plugin && plugin->info()->genericName() == m_stopProcesses->popupMenu()->text( id ) ) {
    Core::getInstance()->doEmitStopButtonPressed( plugin );
    return;
  } else {
    // oops... list has changed in the meantime
    QString str = m_stopProcesses->popupMenu()->text( id );
    for ( plugin = activeProcesses.first(); plugin; plugin = activeProcesses.next() ) {
      if ( plugin->info()->genericName() == str ) {
  Core::getInstance()->doEmitStopButtonPressed( plugin );
        return;
      }
    }
  }
}

void MainWindowShare::slotStopMenuAboutToShow()
{
  QPopupMenu* popup = m_stopProcesses->popupMenu();
  popup->clear();

  int i = 0;
  for ( KDevPlugin* plugin = activeProcesses.first(); plugin; plugin = activeProcesses.next() ) {
    popup->insertItem( plugin->info()->genericName(), i++ );
  }
}

void MainWindowShare::slotShowMenuBar()
{
    if (m_pMainWnd->menuBar()->isVisible()) {
        m_pMainWnd->menuBar()->hide();
    } else {
        m_pMainWnd->menuBar()->show();
    }
    m_pMainWnd->saveMainWindowSettings( KGlobal::config(), "Mainwindow" );
}

void MainWindowShare::slotConfigureNotifications()
{
    KNotifyDialog::configure(m_pMainWnd, "Notification Configuration Dialog");
}

void MainWindowShare::slotSettings()
{
    KDialogBase dlg(KDialogBase::IconList, i18n("Configure KDevelop"),
                    KDialogBase::Help|KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, m_pMainWnd,
                    "customization dialog");
    dlg.setHelp("setup");

    ShellExtension::getInstance()->createGlobalSettingsPage(&dlg);

    KConfig* config = kapp->config();

    config->setGroup("Global Settings Dialog");
    int height = config->readNumEntry( "Height", 600 );
    int width = config->readNumEntry( "Width", 800 );

    dlg.resize( width, height );

    Core::getInstance()->doEmitConfigWidget(&dlg);
    dlg.exec();

    config->setGroup("Global Settings Dialog");
    config->writeEntry( "Height", dlg.size().height() );
    config->writeEntry( "Width", dlg.size().width() );

    ShellExtension::getInstance()->acceptGlobalSettingsPage(&dlg);
}

void MainWindowShare::slotConfigureEditors()
{
    kdDebug(9000) << " *** MainWindowShare::slotConfigureEditors()" << endl;

    KDevPartController * partController = API::getInstance()->partController();
    KParts::Part * part = partController->activePart();

    KTextEditor::ConfigInterface * conf = dynamic_cast<KTextEditor::ConfigInterface*>( part );
    if ( ! conf )
    {
        kdDebug(9000) << "*** No KTextEditor::ConfigInterface for part!" << endl;
        return;
    }

    // show the modal config dialog for this part if it has a ConfigInterface
    conf->configDialog();
    conf->writeConfig();

#ifdef NEED_CONFIGHACK
    // iterate over other instances of this part type and apply configuration
    if( const QPtrList<KParts::Part> * partlist = partController->parts() )
    {
        QPtrListIterator<KParts::Part> it( *partlist );
        while ( KParts::Part* p = it.current() )
        {
            if ( KTextEditor::ConfigInterface * ci = dynamic_cast<KTextEditor::ConfigInterface *>( p ) )
            {
                ci->readConfig();
            }
            ++it;
        }
    }
#endif
}

void MainWindowShare::slotGUICreated( KParts::Part * part )
{
//    kdDebug(9000) << "MainWindowShare::slotGUICreated()" << endl;

    if ( ! part ) return;

    // disable configuration entry if created part is not an editor
    if ( ! dynamic_cast<KTextEditor::ConfigInterface *>( part ) )
    {
        m_configureEditorAction->setEnabled( false );
        return;
    }

    m_configureEditorAction->setEnabled( true );

    // remove the part's merged menu entry
    KAction * action = part->action("set_confdlg"); // name from katepartui.rc
    if ( action )
    {
        kdDebug(9000) << " *** found \"set_confdlg\" action - unplugging" << endl;
        action->unplugAll();
    }

	if ( KAction * action = part->action("file_save") )
	{
		kdDebug(9000) << " *** found \"file_save\" action - disconnecting" << endl;
		disconnect( action, SIGNAL(activated()), 0, 0 );
		connect( action, SIGNAL(activated()), PartController::getInstance(), SLOT(slotSave()) );
	}

	if ( KAction * action = part->action("file_reload") )
	{
		kdDebug(9000) << " *** found \"file_reload\" action - disconnecting" << endl;
		disconnect( action, SIGNAL(activated()), 0, 0 );
		connect( action, SIGNAL(activated()), PartController::getInstance(), SLOT(slotReload()) );
	}
}

// called when OK ar Apply is clicked in the EditToolbar Dialog
void MainWindowShare::slotNewToolbarConfig()
{
  // replug actionlists here...

  m_pMainWnd->applyMainWindowSettings( KGlobal::config(), "Mainwindow" );

//   PartController::getInstance()->reinstallPopups();
}

void MainWindowShare::slotKeyBindings()
{
  KKeyDialog dlg( false, m_pMainWnd );
  QPtrList<KXMLGUIClient> clients = m_pMainWnd->guiFactory()->clients();
  for( QPtrListIterator<KXMLGUIClient> it( clients );
       it.current(); ++it ) {
    dlg.insert( (*it)->actionCollection() );
  }
  if ( dlg.configure() == KKeyDialog::Accepted )
  {
  // this is needed for when we have multiple embedded kateparts and change one of them.
  // it also needs to be done to their views, as they too have actioncollections to update
    if( const QPtrList<KParts::Part> * partlist = PartController::getInstance()->parts() )
    {
        QPtrListIterator<KParts::Part> it( *partlist );
        while ( KParts::Part* part = it.current() )
        {
            if ( KTextEditor::Document * doc = dynamic_cast<KTextEditor::Document*>( part ) )
            {
                doc->reloadXML();

                QPtrList<KTextEditor::View> const & list = doc->views();
                QPtrListIterator<KTextEditor::View> itt( list );
                while( KTextEditor::View * view = itt.current() )
                {
                    view->reloadXML();
                    ++itt;
                }
            }
            ++it;
        }
    }
  }
}

void MainWindowShare::slotConfigureToolbars()
{
  m_pMainWnd->saveMainWindowSettings( KGlobal::config(), "Mainwindow" );
  KEditToolbar dlg( m_pMainWnd->factory() );
  connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
  dlg.exec();
}

void MainWindowShare::contextMenu(QPopupMenu* popup, const Context *)
{
  if ( m_pMainWnd->menuBar()->isVisible() )
    return;

  int id = popup->insertItem( i18n("Show &Menubar"), m_pMainWnd->menuBar(), SLOT(show()) );
  popup->setWhatsThis(id, i18n("<b>Show menubar</b><p>Lets you switch the menubar on/off."));
}

void MainWindowShare::slotActivePartChanged( KParts::Part * part )
{
    m_configureEditorAction->setEnabled( part && dynamic_cast<KTextEditor::Document*>(part) );
}

#include "mainwindowshare.moc"

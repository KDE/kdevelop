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

#include <QCheckBox>
#include <q3vbox.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <QDir>
#include <QRegExp>

#include <knotifyconfigwidget.h>
#include <kaction.h>
#include <kaboutdata.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kparts/mainwindow.h>
#include <kdialog.h>
#include <ktoolbar.h>
#include <kconfig.h>
#include <kkeydialog.h>
#include <kmenubar.h>
#include <kedittoolbar.h>
#include <kbugreport.h>
#include <kurlrequester.h>
#include <kmenu.h>
#include <kiconloader.h>
#include <kxmlguifactory.h>
#include <ktoolbar.h>
#include <ktoolbarpopupaction.h>
#include <ktoggleaction.h>
#include <kpagedialog.h>

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <kparts/partmanager.h>
#include <kdeversion.h>
#include <kdebug.h>
#include <knotifydialog.h>
#include <ksettings/dialog.h>

#include <config.h>
#include <kglobal.h>

#include "documentcontroller.h"
#include "core.h"
#include "kdevmakefrontend.h"
#include "toplevel.h"
#include "kdevplugincontroller.h"

#include "mainwindowshare.h"

#include "shellextension.h"

using namespace MainWindowUtils;

MainWindowShare::MainWindowShare(QObject* pParent)
  :QObject(pParent)
  ,m_toggleMainToolbar(0)
  ,m_toggleBuildToolbar(0)
  ,m_toggleViewToolbar(0)
  ,m_toggleBrowserToolbar(0)
  ,m_toggleStatusbar(0)
  ,m_stopProcesses(0)
{
  m_pMainWnd = (KParts::MainWindow*)pParent;
  m_settingsDialog = 0;

}

void MainWindowShare::init()
{
  connect(Core::getInstance(), SIGNAL(contextMenu(QMenu *, const Context *)),
          this, SLOT(contextMenu(QMenu *, const Context *)));
}

void MainWindowShare::slotActionStatusText( const QString &text )
{
//    kDebug(9000) << "MainWindowShare::slotActionStatusText() - " << text << endl;

    if ( ! m_pMainWnd ) return;

    KStatusBar * statusBar = m_pMainWnd->statusBar();

    if ( ! statusBar ) return;

    statusBar->message( text );
}

void MainWindowShare::createActions()
{
    //FIXME
//   ProjectManager::getInstance()->createActions( m_pMainWnd->actionCollection() );

  KStdAction::quit(this->parent(), SLOT(close()), m_pMainWnd->actionCollection());

  KAction* action;

  m_stopProcesses = new KToolBarPopupAction( i18n( "&Stop" ), "stop",
                                 Qt::Key_Escape, this, SLOT(slotStopButtonPressed()),
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
  action->setToolTip( i18n( "Configure KDevelop" ) );
  action->setWhatsThis(QString("<b>%1</b><p>%2").arg(i18n( "Configure KDevelop" )).arg(i18n("Lets you customize KDevelop.")));

  m_toggleStatusbar = (KToggleAction*) KStdAction::create(KStdAction::ShowToolbar, this, SLOT(slotToggleStatusbar()),m_pMainWnd->actionCollection());
  m_toggleStatusbar->setText(i18n("Show &Statusbar"));
  m_toggleStatusbar->setToolTip( i18n("Show statusbar") );
  m_toggleStatusbar->setWhatsThis(i18n("<b>Show statusbar</b><p>Hides or shows the statusbar."));

  action = new KAction( i18n("&Next Window"), m_pMainWnd->actionCollection(), "view_next_window");
  connect(action, SIGNAL(triggered(bool)), SIGNAL(gotoNextWindow()));
  action->setShortcut(Qt::ALT+Qt::Key_Right);
  action->setToolTip( i18n("Next window") );
  action->setWhatsThis(i18n("<b>Next window</b><p>Switches to the next window."));

  action = new KAction( i18n("&Previous Window"), m_pMainWnd->actionCollection(), "view_previous_window");
  connect(action, SIGNAL(triggered(bool)), SIGNAL(gotoPreviousWindow()));
  action->setShortcut(Qt::ALT+Qt::Key_Left);
  action->setToolTip( i18n("Previous window") );
  action->setWhatsThis(i18n("<b>Previous window</b><p>Switches to the previous window."));

  action = new KAction( i18n("&Last Accessed Window"), m_pMainWnd->actionCollection(), "view_last_window");
  connect(action, SIGNAL(triggered(bool)), SIGNAL(gotoLastWindow()));
  action->setShortcut(Qt::ALT+Qt::Key_Up);
  action->setToolTip( i18n("Last accessed window") );
  action->setWhatsThis(i18n("<b>Last accessed window</b><p>Switches to the last viewed window (Hold the Alt key pressed and walk on by repeating the Up key)."));

  action = new KAction( i18n("&First Accessed Window"), m_pMainWnd->actionCollection(), "view_first_window");
  connect(action, SIGNAL(triggered(bool)), SIGNAL(gotoFirstWindow()));
  action->setShortcut(Qt::ALT+Qt::Key_Down);
  action->setToolTip( i18n("First accessed window") );
  action->setWhatsThis(i18n("<b>First accessed window</b><p>Switches to the first accessed window (Hold the Alt key pressed and walk on by repeating the Down key)."));

  m_configureEditorAction = new KAction( i18n("Configure &Editor..."), m_pMainWnd->actionCollection(), "settings_configure_editors");
  connect(m_configureEditorAction, SIGNAL(triggered(bool)), SLOT( slotConfigureEditors() ));
  m_configureEditorAction->setToolTip( i18n("Configure editor settings") );
  m_configureEditorAction->setWhatsThis(i18n("<b>Configure editor</b><p>Opens editor configuration dialog."));
  m_configureEditorAction->setEnabled( false );

    //FIXME PORT
//   KDevDocumentController * documentController = KDevApi::self()->documentController();
//   connect( documentController, SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotActivePartChanged(KParts::Part* )) );
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
    activeProcesses.removeAll( plugin );
  }
  m_stopProcesses->setEnabled( !activeProcesses.isEmpty() );
}

void MainWindowShare::slotStopPopupActivated( int id )
{
    /* FIXME port
    KDevPlugin* plugin = activeProcesses.at( id );
    if ( plugin && plugin->info()->genericName() == m_stopProcesses->popupMenu()->text( id ) ) {
        Core::getInstance()->doEmitStopButtonPressed( plugin );
        return;
    } else {
        // oops... list has changed in the meantime
        QString str = m_stopProcesses->popupMenu()->text( id );
        for (int i = 0; i < activeProcesses.count(); ++i) {
            plugin = activeProcesses.at(i);
            if ( plugin->info()->genericName() == str ) {
                Core::getInstance()->doEmitStopButtonPressed( plugin );
                return;
            }
        }
    } */
}

void MainWindowShare::slotStopMenuAboutToShow()
{
  /* FIXME port
  QMenu* popup = m_stopProcesses->menu();
  popup->clear();

  int i = 0;
  foreach(KDevPlugin *plugin, activeProcesses) {
    popup->insertItem( plugin->info()->genericName(), i++ );
  }
  */
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
    KNotifyConfigWidget::configure(m_pMainWnd);
}

void MainWindowShare::slotSettings()
{
    if ( !m_settingsDialog )
        m_settingsDialog = new KSettings::Dialog( KSettings::Dialog::Static, m_pMainWnd );

    m_settingsDialog->show();
}

void MainWindowShare::slotConfigureEditors()
{
    kDebug(9000) << " *** MainWindowShare::slotConfigureEditors()" << endl;

    KDevDocumentController * documentController = KDevApi::self()->documentController();
    KDevDocument * document = documentController->activeDocument();

    KTextEditor::Document *doc = document->textDocument();
    KTextEditor::Editor *editor = doc ? doc->editor() : 0;
    if (!editor)
    {
        kDebug(9000) << "*** No KTextEditor::ConfigInterface for part!" << endl;
        return;
    }

    if (!editor->configDialogSupported())
        return;

    // show the modal config dialog for this part if it has a ConfigInterface
    editor->configDialog(m_pMainWnd);
    editor->writeConfig(KGlobal::config());
}

void MainWindowShare::slotGUICreated( KParts::Part * part )
{
//    kDebug(9000) << "MainWindowShare::slotGUICreated()" << endl;

    if ( !part ) return;

    KTextEditor::Document *doc = qobject_cast<KTextEditor::Document *>(part);
    // disable configuration entry if created part is not an editor
    if (!doc || !doc->editor() || !doc->editor()->configDialogSupported())
    {
        m_configureEditorAction->setEnabled( false );
        return;
    }

    m_configureEditorAction->setEnabled( true );

    // remove the part's merged menu entry
    KAction * action = part->action("set_confdlg"); // name from katepartui.rc
    if ( action )
    {
        kDebug(9000) << " *** found \"set_confdlg\" action - unplugging" << endl;
        action->unplugAll();
    }

    if ( KAction * action = part->action("file_save") )
    {
        kDebug(9000) << " *** found \"file_save\" action - disconnecting" << endl;
        disconnect( action, SIGNAL(activated()), 0, 0 );
        connect( action, SIGNAL(activated()), DocumentController::getInstance(), SLOT(slotSave()) );
    }

    if ( KAction * action = part->action("file_reload") )
    {
        kDebug(9000) << " *** found \"file_reload\" action - disconnecting" << endl;
        disconnect( action, SIGNAL(activated()), 0, 0 );
        connect( action, SIGNAL(activated()), DocumentController::getInstance(), SLOT(slotReload()) );
    }
}

// called when OK ar Apply is clicked in the EditToolbar Dialog
void MainWindowShare::slotNewToolbarConfig()
{
  // replug actionlists here...

  m_pMainWnd->applyMainWindowSettings( KGlobal::config(), "Mainwindow" );

//   DocumentController::getInstance()->reinstallPopups();
}

void MainWindowShare::slotKeyBindings()
{
  KKeyDialog dlg( KKeyChooser::AllActions, KKeyChooser::LetterShortcutsDisallowed, m_pMainWnd );
  QList<KXMLGUIClient*> clients = m_pMainWnd->guiFactory()->clients();
  QList<KXMLGUIClient*>::iterator it, itEnd = clients.end();
  for( it = clients.begin(); it != itEnd; ++it ) {
    dlg.insert( (*it)->actionCollection() );
  }
  if ( dlg.configure() == KKeyDialog::Accepted )
  {
      //FIXME is this really needed any longer?
  // this is needed for when we have multiple embedded kateparts and change one of them.
  // it also needs to be done to their views, as they too have actioncollections to update
//     foreach (KParts::Part* part, DocumentController::getInstance()->parts())
//     {
//         if ( KTextEditor::Document * doc = dynamic_cast<KTextEditor::Document*>( part ) )
//         {
//             doc->reloadXML();
// 
//             foreach(KDocument::View *view, doc->views())
//                 view->reloadXML();
//         }
//     }
  }
}

void MainWindowShare::slotConfigureToolbars()
{
  m_pMainWnd->saveMainWindowSettings( KGlobal::config(), "Mainwindow" );
  KEditToolbar dlg( m_pMainWnd->factory() );
  connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
  dlg.exec();
}

void MainWindowShare::contextMenu(QMenu* popup, const Context *)
{
  if ( m_pMainWnd->menuBar()->isVisible() )
    return;

  QAction* action = popup->addAction( i18n("Show &Menubar"), m_pMainWnd->menuBar(), SLOT(show()) );
  action->setWhatsThis(i18n("<b>Show menubar</b><p>Lets you switch the menubar on/off."));
}

void MainWindowShare::slotActivePartChanged( KParts::Part * part )
{
    m_configureEditorAction->setEnabled( part && dynamic_cast<KTextEditor::Document*>(part) );
}

#include "mainwindowshare.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

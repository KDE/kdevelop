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

#include <kaction.h>
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

#if (KDE_VERSION > 305)
#include <knotifydialog.h>
#endif


#include "projectmanager.h"
#include "core.h"
#include "settingswidget.h"
#include "api.h"
#include "kdevmakefrontend.h"

#include "mainwindowshare.h"

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

void MainWindowShare::createActions()
{
  ProjectManager::getInstance()->createActions( m_pMainWnd->actionCollection() );

  KStdAction::quit(this->parent(), SLOT(slotQuit()), m_pMainWnd->actionCollection());

  KAction* action;

  m_stopProcesses = new KAction( i18n( "&Stop" ), "stop",
                                 Key_Escape, Core::getInstance(), SIGNAL(stopButtonClicked()),
                                 m_pMainWnd->actionCollection(), "stop_processes" );
  m_stopProcesses->setStatusText(i18n("Stop all running processes"));
  m_stopProcesses->setEnabled( false );

  connect( Core::getInstance(), SIGNAL(activeProcessCountChanged(uint)),
           this, SLOT(slotActiveProcessCountChanged(uint)) );

  action = KStdAction::showMenubar(
                this, SLOT(slotShowMenuBar()),
                m_pMainWnd->actionCollection(), "settings_show_menubar" );
  action->setStatusText(i18n("Lets you switch the menubar on/off"));

  action = KStdAction::keyBindings(
                this, SLOT(slotKeyBindings()),
                m_pMainWnd->actionCollection(), "settings_configure_shortcuts" );
  action->setStatusText(i18n("Lets you configure shortcut keys"));

  action = KStdAction::configureToolbars(
                this, SLOT(slotConfigureToolbars()),
                m_pMainWnd->actionCollection(), "settings_configure_toolbars" );
  action->setStatusText(i18n("Lets you configure toolbars"));

#if (KDE_VERSION > 305)

  action = KStdAction::configureNotifications(
                this, SLOT(slotConfigureNotifications()),
                m_pMainWnd->actionCollection(), "settings_configure_notifications" );
  action->setStatusText(i18n("Lets you configure system notifications"));
#endif

  action = KStdAction::preferences(this, SLOT(slotSettings()),
                m_pMainWnd->actionCollection(), "settings_configure" );
  action->setStatusText( i18n("Lets you customize KDevelop") );


  m_toggleMainToolbar = KStdAction::showToolbar(this, SLOT(slotToggleMainToolbar()), m_pMainWnd->actionCollection(), "settings_main_toolbar");
  m_toggleMainToolbar->setText(i18n("Show &Main Toolbar"));
  m_toggleMainToolbar->setStatusText( i18n("Hides or shows the main toolbar") );

  m_toggleBuildToolbar = KStdAction::showToolbar(this, SLOT(slotToggleBuildToolbar()),m_pMainWnd->actionCollection(), "settings_build_toolbar");
  m_toggleBuildToolbar->setText(i18n("Show &Build Toolbar"));
  m_toggleBuildToolbar->setStatusText( i18n("Hides or shows the build toolbar") );

  m_toggleViewToolbar = KStdAction::showToolbar(this, SLOT(slotToggleViewToolbar()),m_pMainWnd->actionCollection(), "settings_view_toolbar");
  m_toggleViewToolbar->setText(i18n("Show &View Toolbar"));
  m_toggleViewToolbar->setStatusText( i18n("Hides or shows the view toolbar") );

  m_toggleBrowserToolbar = KStdAction::showToolbar(this, SLOT(slotToggleBrowserToolbar()),m_pMainWnd->actionCollection(), "settings_browser_toolbar");
  m_toggleBrowserToolbar->setText(i18n("Show &Browser Toolbar"));
  m_toggleBrowserToolbar->setStatusText( i18n("Hides or shows the browser toolbar") );

  m_toggleStatusbar = KStdAction::showToolbar(this, SLOT(slotToggleStatusbar()),m_pMainWnd->actionCollection(), "settings_statusbar");
  m_toggleStatusbar->setText(i18n("Show &Statusbar"));
  m_toggleStatusbar->setStatusText( i18n("Hides or shows the statusbar") );

  action = new KAction( i18n("&Next Window"), ALT+Key_Right, this, SIGNAL(gotoNextWindow()),m_pMainWnd->actionCollection(), "view_next_window");
  action->setStatusText( i18n("Switches to the next window") );

  action = new KAction( i18n("&Previous Window"), ALT+Key_Left, this, SIGNAL(gotoPreviousWindow()),m_pMainWnd->actionCollection(), "view_previous_window");
  action->setStatusText( i18n("Switches to the previous window") );

  action = new KAction( i18n("&Last Accessed Window"), ALT+Key_Up, this, SIGNAL(gotoLastWindow()), m_pMainWnd->actionCollection(), "view_last_window");
  action->setStatusText( i18n("Switches to the last viewed window (Hold the Alt key pressed and walk on by repeating the Up key") );

  action = new KAction( i18n("&First Accessed Window"), ALT+Key_Down, this, SIGNAL(gotoFirstWindow()), m_pMainWnd->actionCollection(), "view_first_window");
  action->setStatusText( i18n("Switches to the first accessed window (Hold the Alt key pressed and walk on by repeating the Down key") );

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

void MainWindowShare::slotActiveProcessCountChanged( uint active )
{
  m_stopProcesses->setEnabled( active > 0 );
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
#if (KDE_VERSION > 305)
    KNotifyDialog::configure(m_pMainWnd, "Notification Configuration Dialog");
#endif
}

void MainWindowShare::slotSettings()
{
    KDialogBase dlg(KDialogBase::TreeList, i18n("Customize KDevelop"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, m_pMainWnd,
                    "customization dialog");

    QVBox *vbox = dlg.addVBoxPage(i18n("General"));
    SettingsWidget *gsw = new SettingsWidget(vbox, "general settings widget");

    KConfig* config = kapp->config();
    config->setGroup("General Options");
    gsw->lastProjectCheckbox->setChecked(config->readBoolEntry("Read Last Project On Startup",true));
    config->setGroup("MakeOutputView");
    gsw->setMessageFont(config->readFontEntry("Messages Font"));
    gsw->lineWrappingCheckBox->setChecked(config->readBoolEntry("LineWrapping",true));
    gsw->dirNavigMsgCheckBox->setChecked(config->readBoolEntry("ShowDirNavigMsg",false));
    gsw->compilerOutputButtonGroup->setRadioButtonExclusive(true);
    gsw->compilerOutputButtonGroup->setButton(config->readNumEntry("CompilerOutputLevel",1));
    config->setGroup("General Options");
    gsw->setApplicationFont(config->readFontEntry("Application Font"));
    gsw->changeMessageFontButton->setText(gsw->messageFont().family());
    gsw->changeMessageFontButton->setFont(gsw->messageFont());
    gsw->changeApplicationFontButton->setText(gsw->applicationFont().family());
    gsw->changeApplicationFontButton->setFont(gsw->applicationFont());
    Core::getInstance()->doEmitConfigWidget(&dlg);
    dlg.exec();

    config->setGroup("General Options");
    config->writeEntry("Read Last Project On Startup",gsw->lastProjectCheckbox->isChecked());
    config->writeEntry("Application Font", gsw->applicationFont());
    config->setGroup("MakeOutputView");
    config->writeEntry("Messages Font",gsw->messageFont());
    config->writeEntry("LineWrapping",gsw->lineWrappingCheckBox->isChecked());
    config->writeEntry("ShowDirNavigMsg",gsw->dirNavigMsgCheckBox->isChecked());
    QButton* pSelButton = gsw->compilerOutputButtonGroup->selected();
    config->writeEntry("CompilerOutputLevel",gsw->compilerOutputButtonGroup->id(pSelButton)); // id must be in sync with the enum!
    config->sync();
    API::getInstance()->makeFrontend()->updateSettingsFromConfig();
}

// called when OK ar Apply is clicked in the EditToolbar Dialog
void MainWindowShare::slotNewToolbarConfig()
{
  // replug actionlists here...

  m_pMainWnd->applyMainWindowSettings( KGlobal::config(), "Mainwindow" );
}

void MainWindowShare::slotKeyBindings()
{
  KKeyDialog dlg( false, m_pMainWnd );
  QPtrList<KXMLGUIClient> clients = m_pMainWnd->guiFactory()->clients();
  for( QPtrListIterator<KXMLGUIClient> it( clients );
       it.current(); ++it ) {
    dlg.insert( (*it)->actionCollection() );
  }
  dlg.configure();
}

void MainWindowShare::slotConfigureToolbars()
{
  m_pMainWnd->saveMainWindowSettings( KGlobal::config(), "Mainwindow" );
  KEditToolbar dlg( m_pMainWnd->factory() );
  connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(slotNewToolbarConfig()));
  dlg.exec();
}

#include "mainwindowshare.moc"

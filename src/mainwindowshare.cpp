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

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/configinterface.h>
#include <kparts/partmanager.h>
#include <kdevpartcontroller.h>
#include <kdeversion.h>
#include <kdebug.h>
#include "partcontroller.h"

#if (KDE_VERSION > 305)
#include <knotifydialog.h>
#endif

#include <config.h>

#include "projectmanager.h"
#include "core.h"
#include "settingswidget.h"
#include "api.h"
#include "kdevmakefrontend.h"
#include "toplevel.h"

#include "mainwindowshare.h"

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
    kdDebug(9000) << "MainWindowShare::slotActionStatusText() - " << text << endl;

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

#if (KDE_VERSION > 305)

  action = KStdAction::configureNotifications(
                this, SLOT(slotConfigureNotifications()),
                m_pMainWnd->actionCollection(), "settings_configure_notifications" );
  action->setToolTip(beautifyToolTip(action->text()));
  action->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(action->text())).arg(i18n("Lets you configure system notifications.")));
#endif

  action = KStdAction::preferences(this, SLOT(slotSettings()),
                m_pMainWnd->actionCollection(), "settings_configure" );
  action->setToolTip(beautifyToolTip(action->text()));
  action->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(action->text())).arg(i18n("Lets you customize KDevelop.")));


#if (KDE_VERSION <= 305)
  m_toggleMainToolbar = KStdAction::showToolbar(this, SLOT(slotToggleMainToolbar()), m_pMainWnd->actionCollection(), "settings_main_toolbar");
  m_toggleMainToolbar->setText(i18n("Show &Main Toolbar"));
  m_toggleMainToolbar->setToolTip( i18n("Show Main Toolbar") );
  m_toggleMainToolbar->setWhatsThis(i18n("<b>Show Main Toolbar</b><p>Hides or shows the main toolbar."));

  m_toggleBuildToolbar = KStdAction::showToolbar(this, SLOT(slotToggleBuildToolbar()),m_pMainWnd->actionCollection(), "settings_build_toolbar");
  m_toggleBuildToolbar->setText(i18n("Show &Build Toolbar"));
  m_toggleBuildToolbar->setToolTip( i18n("Show Build Toolbar") );
  m_toggleBuildToolbar->setWhatsThis(i18n("<b>Show Build Toolbar</b><p>Hides or shows the build toolbar."));

  m_toggleViewToolbar = KStdAction::showToolbar(this, SLOT(slotToggleViewToolbar()),m_pMainWnd->actionCollection(), "settings_view_toolbar");
  m_toggleViewToolbar->setText(i18n("Show &View Toolbar"));
  m_toggleViewToolbar->setToolTip( i18n("Show View Toolbar") );
  m_toggleViewToolbar->setWhatsThis(i18n("<b>Show View Toolbar</b><p>Hides or shows the view toolbar."));

  m_toggleBrowserToolbar = KStdAction::showToolbar(this, SLOT(slotToggleBrowserToolbar()),m_pMainWnd->actionCollection(), "settings_browser_toolbar");
  m_toggleBrowserToolbar->setText(i18n("Show &Browser Toolbar"));
  m_toggleBrowserToolbar->setToolTip( i18n("Show Browser Toolbar") );
  m_toggleBrowserToolbar->setWhatsThis(i18n("<b>Show Browser Toolbar</b><p>Hides or shows the browser toolbar."));
#endif

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
  if ( plugin && plugin->pluginName() == m_stopProcesses->popupMenu()->text( id ) ) {
    Core::getInstance()->doEmitStopButtonPressed( plugin );
    return;
  } else {
    // oops... list has changed in the meantime
    QString str = m_stopProcesses->popupMenu()->text( id );
    for ( plugin = activeProcesses.first(); plugin; plugin = activeProcesses.next() ) {
      if ( plugin->pluginName() == str ) {
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
    popup->insertItem( plugin->pluginName(), i++ );
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
#if (KDE_VERSION > 305)
    KNotifyDialog::configure(m_pMainWnd, "Notification Configuration Dialog");
#endif
}

void MainWindowShare::slotSettings()
{
    KDialogBase dlg(KDialogBase::TreeList, i18n("Configure KDevelop"),
                    KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok, m_pMainWnd,
                    "customization dialog");

    SettingsWidget *gsw;
    KConfig* config = kapp->config();
    if (TopLevel::mode != TopLevel::AssistantMode)
    {
        QVBox *vbox = dlg.addVBoxPage(i18n("General"));
        gsw = new SettingsWidget(vbox, "general settings widget");

        gsw->projects_url->setMode((int)KFile::Directory);

        config->setGroup("General Options");
        gsw->lastProjectCheckbox->setChecked(config->readBoolEntry("Read Last Project On Startup",true));
        config->setGroup("MakeOutputView");
        gsw->setMessageFont(config->readFontEntry("Messages Font"));
        gsw->lineWrappingCheckBox->setChecked(config->readBoolEntry("LineWrapping",true));
        gsw->dirNavigMsgCheckBox->setChecked(config->readBoolEntry("ShowDirNavigMsg",false));
        gsw->compilerOutputButtonGroup->setRadioButtonExclusive(true);
        gsw->compilerOutputButtonGroup->setButton(config->readNumEntry("CompilerOutputLevel",0));
        config->setGroup("General Options");
        gsw->setApplicationFont(config->readFontEntry("Application Font"));
        gsw->changeMessageFontButton->setText(gsw->messageFont().family());
        gsw->changeMessageFontButton->setFont(gsw->messageFont());
        gsw->changeApplicationFontButton->setText(gsw->applicationFont().family());
        gsw->changeApplicationFontButton->setFont(gsw->applicationFont());
        gsw->projects_url->setURL(config->readPathEntry("DefaultProjectsDir", QDir::homeDirPath()+"/"));
    }

    config->setGroup("Global Settings Dialog");
    int height = config->readNumEntry( "Height", 600 );
    int width = config->readNumEntry( "Width", 800 );
    
    dlg.resize( width, height );
  
    Core::getInstance()->doEmitConfigWidget(&dlg);
    dlg.exec();

    config->setGroup("Global Settings Dialog");
    config->writeEntry( "Height", dlg.size().height() );
    config->writeEntry( "Width", dlg.size().width() );

    if (TopLevel::mode != TopLevel::AssistantMode)
    {
        config->setGroup("General Options");
        config->writeEntry("Read Last Project On Startup",gsw->lastProjectCheckbox->isChecked());
#if defined(KDE_IS_VERSION)
# if KDE_IS_VERSION(3,1,3)
#  ifndef _KDE_3_1_3_
#   define _KDE_3_1_3_
#  endif
# endif
#endif
#if defined(_KDE_3_1_3_)
        config->writePathEntry("DefaultProjectsDir", gsw->projects_url->url());
#else
        config->writeEntry("DefaultProjectsDir", gsw->projects_url->url());
#endif
        config->writeEntry("Application Font", gsw->applicationFont());
        config->setGroup("MakeOutputView");
        config->writeEntry("Messages Font",gsw->messageFont());
        config->writeEntry("LineWrapping",gsw->lineWrappingCheckBox->isChecked());
        config->writeEntry("ShowDirNavigMsg",gsw->dirNavigMsgCheckBox->isChecked());
        QButton* pSelButton = gsw->compilerOutputButtonGroup->selected();
        config->writeEntry("CompilerOutputLevel",gsw->compilerOutputButtonGroup->id(pSelButton)); // id must be in sync with the enum!
        config->sync();
        if( API::getInstance()->makeFrontend() )
            API::getInstance()->makeFrontend()->updateSettingsFromConfig();
    }
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
}

// called when OK ar Apply is clicked in the EditToolbar Dialog
void MainWindowShare::slotNewToolbarConfig()
{
  // replug actionlists here...

  m_pMainWnd->applyMainWindowSettings( KGlobal::config(), "Mainwindow" );

  PartController::getInstance()->reinstallPopups();
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

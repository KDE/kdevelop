/* This file is part of the KDevelop project
Copyright 2002 Falk Brettschneider <falkbr@kdevelop.org>
Copyright 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright 2006 Adam Treat <treat@kde.org>
Copyright 2006, 2007 Alexander Dymo <adymo@kdevelop.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#include "mainwindow_p.h"

#include <QApplication>
#include <QStackedWidget>

#include <kmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <knotifyconfigwidget.h>

#include <kparts/part.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>

#include <kstandardaction.h>
#include <kselectaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <ktoolbarpopupaction.h>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/document.h>

#include "iplugin.h"

#include "core.h"
#include "partdocument.h"
#include "partcontroller.h"
#include "uicontroller.h"
#include "statusbar.h"

#include "mainwindow.h"

namespace KDevelop {

bool MainWindowPrivate::s_quitRequested = false;

MainWindowPrivate::MainWindowPrivate(MainWindow *mainWindow)
: m_mainWindow(mainWindow), m_statusBar(new KDevelop::StatusBar(mainWindow)), lastXMLGUIClientView(0)
{
    m_mainWindow->setStatusBar(m_statusBar);
}

void MainWindowPrivate::addPlugin( IPlugin *plugin )
{
    kDebug() << "add plugin" << plugin << plugin->componentData().componentName();
    Q_ASSERT( plugin );

    m_mainWindow->guiFactory()->addClient( plugin );
}

void MainWindowPrivate::removePlugin( IPlugin *plugin )
{
    Q_ASSERT( plugin );

    m_mainWindow->guiFactory()->removeClient( plugin );
}

void MainWindowPrivate::activePartChanged(KParts::Part *part)
{
    if ( Core::self()->uiController()->activeMainWindow() == m_mainWindow)
        m_mainWindow->createGUI(part);
}

void MainWindowPrivate::changeActiveView(Sublime::View *view)
{
    // If the previous view was KXMLGUIClient, remove its actions
    // In the case that that view was removed, lastActiveView
    // will auto-reset, and xmlguifactory will disconnect that
    // client, I think.
    if (lastXMLGUIClientView)
    {
        kDebug() << "clearing last XML GUI client" << lastXMLGUIClientView;
        m_mainWindow->guiFactory()->removeClient(
            dynamic_cast<KXMLGUIClient*>(lastXMLGUIClientView));
        disconnect (lastXMLGUIClientView, SIGNAL(destroyed(QObject*)), this, 0);
        lastXMLGUIClientView = NULL;
    }

    if (!view)
        return;

    QWidget* viewWidget = view->widget();
    Q_ASSERT(viewWidget);
    
    kDebug() << "changing active view to" << view << "doc" << view->document() << "mw" << m_mainWindow;

    IDocument *doc = dynamic_cast<KDevelop::IDocument*>(view->document());
    if (doc)
    {
        //activate part if it is not yet activated
        doc->activate(view, m_mainWindow);
    }
    else
    {
        //activated view is not a part document so we need to remove active part gui
        ///@todo adymo: only this window needs to remove GUI
//         KParts::Part *activePart = Core::self()->partManager()->activePart();
//         if (activePart)
//             guiFactory()->removeClient(activePart);
    }

    // If the new view is KXMLGUIClient, add it.
    if (KXMLGUIClient* c = dynamic_cast<KXMLGUIClient*>(viewWidget))
    {
        lastXMLGUIClientView = viewWidget;
        m_mainWindow->guiFactory()->addClient(c);
        connect(viewWidget, SIGNAL(destroyed(QObject*)), 
                this, SLOT(xmlguiclientDestroyed(QObject*)));
    }
}

void MainWindowPrivate::xmlguiclientDestroyed(QObject* obj)
{
    /* We're informed the the QWidget for the active view that is also
       KXMLGUIclient is dying.  KXMLGUIFactory will not like deleted
       clients, really.  Unfortunately, there's nothing we can do
       at this point. For example, KateView derives from QWidget and
       KXMLGUIClient.  The destroyed() signal is emitted by ~QWidget.
       At this point, event attempt to cross-cast to KXMLGUIClient
       is undefined behaviour.  We hope to catch view deletion a bit
       later, but if we fail, we better report it now, rather than
       get a weird crash a bit later.  */
       Q_ASSERT(false && "xmlgui clients management is messed up");
}

void MainWindowPrivate::setupActions()
{
    KStandardAction::quit( this, SLOT( quitAll() ), actionCollection() );

    KAction *action;

    QString app = qApp->applicationName();
    QString text = i18n( "Configure %1", app );
    action = KStandardAction::preferences( this, SLOT( settingsDialog() ),
                                      actionCollection());
    actionCollection()->addAction( "settings_configure", action );
    action->setToolTip( text );
    action->setWhatsThis( QString( "<b>%1</b><p>%2<p>" ).arg( text ).arg(
                              i18n( "Lets you customize %1.", app ) ) );

    action = actionCollection()->addAction( "settings_configure_editors" );
    action->setText( i18n( "Configure &Editor..." ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( configureEditors() ) );
    action->setToolTip( i18n( "Configure editor settings" ) );
    action->setWhatsThis( i18n( "<b>Configure editor</b><p>Opens editor configuration dialog.</p>" ) );
    action->setEnabled( false );

    action = KStandardAction::showStatusbar( this, SLOT( toggleStatusbar() ),
                                        actionCollection());
    actionCollection()->addAction( "settings_show_statusbar", action );
    action->setText( i18n( "Show &Statusbar" ) );
    action->setToolTip( i18n( "Show statusbar" ) );
    action->setWhatsThis( i18n( "<b>Show statusbar</b><p>Hides or shows the statusbar.</p>" ) );

//     KToolBarPopupAction *popupAction;
//     popupAction = new KToolBarPopupAction( KIcon( "process-stop" ),
//                                            i18n( "&Stop" ),
//                                            actionCollection()
//                                            );
//     actionCollection()->addAction( "stop_processes",popupAction );

//     popupAction->setShortcut( Qt::Key_Escape );
//     popupAction->setToolTip( i18n( "Stop" ) );
//     popupAction->setWhatsThis( i18n( "<b>Stop</b><p>Stops all running processes." ) );
//     popupAction->setEnabled( false );
//     connect( popupAction, SIGNAL( triggered() ),
//              this, SLOT( stopButtonPressed() ) );
//     connect( popupAction->menu(), SIGNAL( aboutToShow() ),
//              this, SLOT( stopMenuAboutToShow() ) );
//     connect( popupAction->menu(), SIGNAL( triggered( Action* ) ),
//              this, SLOT( stopPopupActivated( QAction* ) ) );

    action = KStandardAction::showMenubar(
                 this, SLOT( showMenuBar() ),
                 actionCollection());
    actionCollection()->addAction( "settings_show_menubar", action );
    action->setToolTip( action->text() );
    action->setWhatsThis( QString( "<b>%1</b><p>%2<p>" ).arg( action->text() ).arg( i18n( "Lets you toggle the menubar on/off." ) ) );

    actionCollection()->addAction( KStandardAction::KeyBindings, m_mainWindow->guiFactory(), SLOT( configureShortcuts() ) );

    action = KStandardAction::configureToolbars(
                 this, SLOT( configureToolbars() ),
                 actionCollection() );
    actionCollection()->addAction( "settings_configure_toolbars", action );
    action->setToolTip( action->text() );
    action->setWhatsThis( QString( "<b>%1</b><p>%2<p>" ).arg( action->text() ).arg( i18n( "Lets you configure toolbars." ) ) );

    action = KStandardAction::configureNotifications(
                 this, SLOT( configureNotifications() ),
                 actionCollection());
    actionCollection()->addAction( "settings_configure_notifications",action );
    action->setToolTip( action->text() );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( action->text() ).arg( i18n( "Lets you configure system notifications." ) ) );

    action = actionCollection()->addAction( "view_next_window" );
    action->setText( i18n( "&Next Window" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoNextWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Right );
    action->setToolTip( i18n( "Next window" ) );
    action->setWhatsThis( i18n( "<b>Next window</b><p>Switches to the next window.</p>" ) );
    action->setIcon(KIcon("go-next"));

    action = actionCollection()->addAction( "view_previous_window" );
    action->setText( i18n( "&Previous Window" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoPreviousWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Left );
    action->setToolTip( i18n( "Previous window" ) );
    action->setWhatsThis( i18n( "<b>Previous window</b><p>Switches to the previous window.</p>" ) );
    action->setIcon(KIcon("go-previous"));

    action = actionCollection()->addAction( "view_last_window" );
    action->setText( i18n( "&Last Accessed Window" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoLastWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Up );
    action->setToolTip( i18n( "Last accessed window" ) );
    action->setWhatsThis( i18n( "<b>Last accessed window</b><p>Switches to the last viewed window (Hold the Alt key pressed and walk on by repeating the Up key).</p>" ) );

    action = actionCollection()->addAction( "view_first_window" );
    action->setText( i18n( "&First Accessed Window" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoFirstWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Down );
    action->setToolTip( i18n( "First accessed window" ) );
    action->setWhatsThis( i18n( "<b>First accessed window</b><p>Switches to the first accessed window (Hold the Alt key pressed and walk on by repeating the Down key).</p>" ) );

    action = actionCollection()->addAction( "new_window" );
    action->setIcon(KIcon( "window-new" ));
    action->setText( i18n( "&New Window" ) );
    action->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_N );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( newWindow() ) );
    action->setToolTip( i18n( "New Window" ) );
    action->setWhatsThis( i18n( "<b>New Window</b><p>Creates a new window with a duplicate of current area.</p>" ) );

    action = actionCollection()->addAction( "split_horizontal" );
    action->setIcon(KIcon( "view-split-top-bottom" ));
    action->setText( i18n( "Split View &Top/Bottom" ) );
    action->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_T );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( splitHorizontal() ) );
    action->setToolTip( i18n( "Split Horizontal" ) );
    action->setWhatsThis( i18n( "<b>Split Horizontal</b><p>Splitts the current view horizontally.</p>" ) );

    action = actionCollection()->addAction( "split_vertical" );
    action->setIcon(KIcon( "view-split-left-right" ));
    action->setText( i18n( "Split View &Left/Right" ) );
    action->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_L );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( splitVertical() ) );
    action->setToolTip( i18n( "Split Vertical" ) );
    action->setWhatsThis( i18n( "<b>Split Vertical</b><p>Splitts the current view vertically.</p>" ) );

    action = actionCollection()->addAction( "file_new" );
    action->setIcon(KIcon("document-new"));
    action->setShortcut( Qt::CTRL + Qt::Key_N );
    action->setText( i18n( "&New File" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( fileNew() ) );
    action->setToolTip( i18n( "New File" ) );
    action->setWhatsThis( i18n( "<b>New File</b><p>Creates an empty file.</p>" ) );

    action = actionCollection()->addAction( "add_toolview" );
    action->setIcon(KIcon("window-new"));
    action->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_V );
    action->setText( i18n( "&Add Tool View..." ) );
    connect( action, SIGNAL( triggered( bool ) ),  SLOT( viewAddNewToolView() ) );
    action->setToolTip( i18n( "Add Tool View" ) );
    action->setWhatsThis( i18n( "<b>Add Tool View</b><p>Adds a new tool view to this window.</p>" ) );
}

KActionCollection * MainWindowPrivate::actionCollection()
{
    return m_mainWindow->actionCollection();
}

/* Make sure that the main toolbar has only items from our actionCollection
   and not from anywhere else.  Presently, when new XMLGUI client is added,
   it's actions are always merged, and there's no clear way to stop this.
   So, we execute the below code whenever a new client is added.  */
void MainWindowPrivate::fixToolbar()
{
    QWidget* w = m_mainWindow->guiFactory()->container(
        "mainToolBar", m_mainWindow);
    if (w)
        foreach (QAction *a, w->actions())
        {
            if (m_mainWindow->actionCollection()->action(a->objectName()) != a)
            {
                w->removeAction(a);
            }
        }
}

bool MainWindowPrivate::applicationQuitRequested() const
{
    return s_quitRequested;
}

}

#include "mainwindow_actions.cpp"
#include "mainwindow_p.moc"


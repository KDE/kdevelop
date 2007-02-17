/* This file is part of the KDevelop project
Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
Copyright (C) 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2006 Alexander Dymo <adymo@kdevelop.org>

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
#include "mainwindow.h"

#include <QHash>
#include <QDockWidget>
#include <QStackedWidget>
#include <QApplication>

#include <kmenu.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <ktemporaryfile.h>

#include <kparts/part.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>

#include <kstandardaction.h>
#include <kselectaction.h>
#include <ktoggleaction.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kactioncollection.h>
#include <ktoolbarpopupaction.h>

#include <knotifyconfigwidget.h>

#include <sublime/area.h>
#include <sublime/view.h>
#include <sublime/document.h>

#include "core.h"
// #include "kdevconfig.h"
#include "iplugin.h"
// #include "kdevprofile.h"
// #include "kdevdocument.h"
// #include "kdevstatusbar.h"
#include "shellextension.h"
// #include "kdevprofileengine.h"
#include "plugincontroller.h"
#include "partcontroller.h"
#include "partdocument.h"
#include "uicontroller.h"
// #include "kdevprojectcontroller.h"
// #include "kdevdocumentcontroller.h"

namespace KDevelop
{


class MainWindowPrivate
{
public:
    MainWindowPrivate()
            :center( 0 )
    {}

    QStackedWidget *center;
    QPointer<QWidget> centralPlugin;

//     QList<Plugin*> activeProcesses;
};

MainWindow::MainWindow( Sublime::Controller *parent, Qt::WFlags flags )
        : Sublime::MainWindow( parent, flags )
{
    setObjectName( QLatin1String( "MainWindow" ) );
    d = new MainWindowPrivate();
    d->center = new QStackedWidget( this );
    setCentralWidget( d->center );

    setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

    setStandardToolBarMenuEnabled( true );
    setupActions();
//     setStatusBar( new KDevelop::StatusBar( this ) );

    setXMLFile( ShellExtension::getInstance() ->xmlFile() );
}

MainWindow::~ MainWindow()
{}

void MainWindow::setupActions()
{
    KStandardAction::quit( this, SLOT( close() ), actionCollection() );

    QAction *action;

    QString app = qApp->applicationName();
    QString text = i18n( "Configure %1", app );
//     action = KStandardAction::preferences( this, SLOT( settingsDialog() ),
//                                       actionCollection());
//     actionCollection()->addAction( "settings_configure", action );
//     action->setToolTip( text );
//     action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( text ).arg(
//                               i18n( "Lets you customize %1.", app ) ) );

    action = actionCollection()->addAction( "settings_configure_editors" );
    action->setText( i18n( "Configure &Editor..." ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( configureEditors() ) );
    action->setToolTip( i18n( "Configure editor settings" ) );
    action->setWhatsThis( i18n( "<b>Configure editor</b><p>Opens editor configuration dialog." ) );
    action->setEnabled( false );

    action = KStandardAction::showStatusbar( this, SLOT( toggleStatusbar() ),
                                        actionCollection());
    actionCollection()->addAction( "settings_show_statusbar", action );
    action->setText( i18n( "Show &Statusbar" ) );
    action->setToolTip( i18n( "Show statusbar" ) );
    action->setWhatsThis( i18n( "<b>Show statusbar</b><p>Hides or shows the statusbar." ) );

    KToolBarPopupAction *popupAction;
    popupAction = new KToolBarPopupAction( KIcon( "stop" ),
                                           i18n( "&Stop" ),
                                           actionCollection()
                                           );
    actionCollection()->addAction( "stop_processes",popupAction );

    popupAction->setShortcut( Qt::Key_Escape );
    popupAction->setToolTip( i18n( "Stop" ) );
    popupAction->setWhatsThis( i18n( "<b>Stop</b><p>Stops all running processes." ) );
    popupAction->setEnabled( false );
    connect( popupAction, SIGNAL( triggered() ),
             this, SLOT( stopButtonPressed() ) );
    connect( popupAction->menu(), SIGNAL( aboutToShow() ),
             this, SLOT( stopMenuAboutToShow() ) );
    connect( popupAction->menu(), SIGNAL( activated( int ) ),
             this, SLOT( stopPopupActivated( int ) ) );

    //FIXME fix connection after gutting of Core
    /*connect( Core::getInstance(), SIGNAL( activeProcessChanged( Plugin*, bool ) ),
    this, SLOT( activeProcessChanged( Plugin*, bool ) ) );*/

    action = KStandardAction::showMenubar(
                 this, SLOT( showMenuBar() ),
                 actionCollection());
    actionCollection()->addAction( "settings_show_menubar", action );
    action->setToolTip( action->text() );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( action->text() ).arg( i18n( "Lets you toggle the menubar on/off." ) ) );

    action = KStandardAction::keyBindings(
                 this, SLOT( keyBindings() ),
                 actionCollection());
    actionCollection()->addAction( "settings_configure_shortcuts", action );
    action->setToolTip( action->text() );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( action->text() ).arg( i18n( "Lets you configure shortcut keys." ) ) );

    action = KStandardAction::configureToolbars(
                 this, SLOT( configureToolbars() ),
                 actionCollection() );
    actionCollection()->addAction( "settings_configure_toolbars", action );
    action->setToolTip( action->text() );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( action->text() ).arg( i18n( "Lets you configure toolbars." ) ) );

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
    action->setWhatsThis( i18n( "<b>Next window</b><p>Switches to the next window." ) );

    action = actionCollection()->addAction( "view_previous_window" );
    action->setText( i18n( "&Previous Window" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoPreviousWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Left );
    action->setToolTip( i18n( "Previous window" ) );
    action->setWhatsThis( i18n( "<b>Previous window</b><p>Switches to the previous window." ) );

    action = actionCollection()->addAction( "view_last_window" );
    action->setText( i18n( "&Last Accessed Window" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoLastWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Up );
    action->setToolTip( i18n( "Last accessed window" ) );
    action->setWhatsThis( i18n( "<b>Last accessed window</b><p>Switches to the last viewed window (Hold the Alt key pressed and walk on by repeating the Up key)." ) );

    action = actionCollection()->addAction( "view_first_window" );
    action->setText( i18n( "&First Accessed Window" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoFirstWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Down );
    action->setToolTip( i18n( "First accessed window" ) );
    action->setWhatsThis( i18n( "<b>First accessed window</b><p>Switches to the first accessed window (Hold the Alt key pressed and walk on by repeating the Down key)." ) );

    action = actionCollection()->addAction( "new_window" );
    action->setText( i18n( "&New Window" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( newWindow() ) );
    action->setToolTip( i18n( "New Window" ) );
    action->setWhatsThis( i18n( "<b>New Window</b><p>Creates a new window with a duplicate of current area." ) );

    action = actionCollection()->addAction( "split_horizontal" );
    action->setIcon(KIcon( "split_h" ));
    action->setText( i18n( "Split &Horizontal" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( splitHorizontal() ) );
    action->setToolTip( i18n( "Split Horizontal" ) );
    action->setWhatsThis( i18n( "<b>Split Horizontal</b><p>Splitts the current view horizontally." ) );

    action = actionCollection()->addAction( "split_vertical" );
    action->setIcon(KIcon( "split_v" ));
    action->setText( i18n( "Split &Vertical" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( splitVertical() ) );
    action->setToolTip( i18n( "Split Vertical" ) );
    action->setWhatsThis( i18n( "<b>Split Vertical</b><p>Splitts the current view vertically." ) );

    action = actionCollection()->addAction( "file_new" );
    action->setIcon(KIcon("filenew"));
    action->setShortcut( Qt::CTRL + Qt::Key_N );
    action->setText( i18n( "&New File" ) );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( fileNew() ) );
    action->setToolTip( i18n( "New File" ) );
    action->setWhatsThis( i18n( "<b>New File</b><p>Creates an empty file." ) );
}


void MainWindow::loadSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded );
/*    KConfig * config = Config::standard();
    applyMainWindowSettings( config, QLatin1String( "MainWindow" ) );*/
}

void MainWindow::saveSettings( bool projectIsLoaded )
{
    Q_UNUSED( projectIsLoaded );
/*    if ( projectIsLoaded )
        return;

    KConfig * config = Config::standard();
    saveMainWindowSettings( config, QLatin1String( "MainWindow" ) );*/
}

void MainWindow::initialize()
{
    createGUI(0);
    Core::self()->partController()->addManagedTopLevelWidget(this);
    connect( Core::self()->pluginController(), SIGNAL(pluginLoaded(IPlugin*)),
             this, SLOT(addPlugin(IPlugin*)));
    connect( Core::self()->partController(), SIGNAL(activePartChanged(KParts::Part*)),
        this, SLOT(activePartChanged(KParts::Part*)));
    connect( this, SIGNAL(activeViewChanged(Sublime::View*)),
        this, SLOT(changeActiveView(Sublime::View*)));
/*    connect( Core::documentController(), SIGNAL( documentActivated( Document* ) ),
             this, SLOT( documentActivated( Document* ) ) );
    connect( Core::projectController(), SIGNAL( projectOpened() ),
             this, SLOT( projectOpened() ) );
    connect( Core::projectController(), SIGNAL( projectClosed() ),
             this, SLOT( projectClosed() ) );*/
}

void MainWindow::cleanup()
{
}

void MainWindow::fillContextMenu( KMenu *menu, const Context *context )
{
    //Perhaps we get rid of this framework and instead have every Context contains
    //a kactioncollection.  Plugins could add their actions directly to the context
    //object retrieved from Core... ??
    emit contextMenu( menu, context );

    //Put this in every context menu so that plugins will be encouraged to allow shortcuts
    QAction * action = actionCollection() ->action( "settings_configure_shortcuts" );
    menu->addAction( action );
}

void MainWindow::addPlugin( IPlugin *plugin )
{
    kDebug(9037) << "add plugin" << endl;
    Q_ASSERT( plugin );

    guiFactory()->addClient( plugin );
}

void MainWindow::removePlugin( IPlugin *plugin )
{
    Q_ASSERT( plugin );

    guiFactory()->removeClient( plugin );
}

void MainWindow::setVisible( bool visible )
{
    KMainWindow::setVisible( visible );
    emit finishedLoading();
}

void MainWindow::gotoNextWindow()
{
    if ( ( d->center->currentIndex() + 1 ) < d->center->count() )
        d->center->setCurrentIndex( d->center->currentIndex() + 1 );
    else
        d->center->setCurrentIndex( 0 );
}

void MainWindow::gotoPreviousWindow()
{
    if ( ( d->center->currentIndex() - 1 ) >= 0 )
        d->center->setCurrentIndex( d->center->currentIndex() - 1 );
    else
        d->center->setCurrentIndex( d->center->count() - 1 );
}

void MainWindow::gotoFirstWindow()
{
    d->center->setCurrentIndex( 0 );
}

void MainWindow::gotoLastWindow()
{
    d->center->setCurrentIndex( d->center->count() - 1 );
}

void MainWindow::projectOpened()
{
    QString app = i18n( "Project" );
    QString text = i18n( "Configure %1", app );
    QAction *action = actionCollection() ->action( "settings_configure" );
    action->setToolTip( text );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( text ).arg(
                              i18n( "Lets you customize %1.", app ) ) );
}

void MainWindow::projectClosed()
{
    QString app = qApp->applicationName();
    QString text = i18n( "Configure %1", app );
    QAction *action = actionCollection() ->action( "settings_configure" );
    action->setToolTip( text );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( text ).arg(
                              i18n( "Lets you customize %1.", app ) ) );
}

void MainWindow::configureToolbars()
{}

void MainWindow::newToolbarConfig()
{
    applyMainWindowSettings( KGlobal::config().data(), "MainWindow" );
}

bool MainWindow::queryClose()
{
    return true;
}

void MainWindow::setupWindowMenu()
{
    //FIXME This should setup a window menu or perhaps dialog instead.
    // Either way, we need one with a scroll bar.  I'm tired of menus that take
    // up the entire screen.
}

void MainWindow::fillWindowMenu()
{
    //FIXME This should fill a window menu or perhaps dialog instead.
    // Either way, we need one with a scroll bar.  I'm tired of menus that take
    // up the entire screen.
}

void MainWindow::reportBug()
{}

void MainWindow::toggleStatusbar()
{
    KToggleAction * action =
        qobject_cast< KToggleAction*>( actionCollection() ->action( "settings_show_statusbar" ) );
    statusBar() ->setHidden( !action->isChecked() );
}

void MainWindow::stopButtonPressed()
{}

void MainWindow::activeProcessChanged( IPlugin* plugin, bool active )
{
    Q_UNUSED( plugin );
    Q_UNUSED( active );
}

void MainWindow::stopPopupActivated( int id )
{
    Q_UNUSED( id );
}

void MainWindow::stopMenuAboutToShow()
{}

void MainWindow::showMenuBar()
{}

void MainWindow::configureNotifications()
{
    KNotifyConfigWidget::configure( this, "Notification Configuration Dialog" );
}


void MainWindow::settingsDialog()
{
//     Config::settingsDialog();
}

void MainWindow::configureEditors()
{
    //FIXME Change this so that it is embedded in our config dialog.
    //Perhaps this will require a change to the KTextEditor interface too...
/*    KTextEditor::Document * doc =
        Core::documentController() ->activeDocument() ->textDocument();
    KTextEditor::Editor *editor = doc ? doc->editor() : 0;
    if ( !editor )
    {
        return ;
    }

    if ( !editor->configDialogSupported() )
    {
        kDebug( 9000 ) << "KTextEditor::configDialogSupported() == false" << endl;
    }

    // show the modal config dialog for this part if it has a ConfigInterface
    editor->configDialog( this );*/
}

void MainWindow::keyBindings()
{}

void MainWindow::newWindow()
{
    Core::self()->uiControllerInternal()->switchToArea(area()->objectName(), UiController::NewWindow);
}

void MainWindow::splitHorizontal()
{
    split(Qt::Horizontal);
}

void MainWindow::splitVertical()
{
    split(Qt::Vertical);
}

void MainWindow::split(Qt::Orientation orientation)
{
    if (!area())
        return;
    Sublime::View *view = activeView();
    if (!view)
        return;

    area()->addView(view->document()->createView(), view, orientation);
}

void KDevelop::MainWindow::activePartChanged(KParts::Part *part)
{
    if ( Core::self()->uiControllerInternal()->activeMainWindow() == this)
        createGUI(part);
}

void MainWindow::fileNew()
{
    Core::self()->uiControllerInternal()->openEmptyDocument();
}

void MainWindow::changeActiveView(Sublime::View *view)
{
    PartDocument *doc = qobject_cast<KDevelop::PartDocument*>(view->document());
    if (doc)
    {
        //activate part if it is not yet activated
        KParts::Part *part = doc->partForWidget(view->widget());
        if (Core::self()->partController()->activePart() != part)
            Core::self()->partController()->setActivePart(part);
    }
    else
    {
        //activated view is not a part document so we need to remove active part gui
        ///@todo adymo: only this window needs to remove GUI
//         KParts::Part *activePart = Core::self()->partController()->activePart();
//         if (activePart)
//             guiFactory()->removeClient(activePart);
    }
}

}

#include "mainwindow.moc"

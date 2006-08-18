/* This file is part of the KDE project
Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
Copyright (C) 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright (C) 2006 Adam Treat <treat@kde.org>

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
#include "kdevmainwindow.h"

#include <QDockWidget>
#include <QStackedWidget>

#include <kmenu.h>
#include <klocale.h>

#include <kparts/part.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editor.h>

#include <kstdaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <ktoolbarpopupaction.h>

#include <knotifydialog.h>
#include <ksettings/dialog.h>

#include "kdevconfig.h"
#include "kdevprofile.h"
#include "kdevprofileengine.h"

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevstatusbar.h"
#include "shellextension.h"
#include "kdevplugincontroller.h"
#include "kdevprojectcontroller.h"
#include "kdevdocumentcontroller.h"

class KDevMainWindowPrivate
{
public:
    KDevMainWindowPrivate()
            : center( 0 ),
            settingsDialog( 0 )
    {}

    QStackedWidget *center;
    KSettings::Dialog *settingsDialog;

    QList<QDockWidget*> dockList;
    QList<KDevPlugin*> activeProcesses;
};

KDevMainWindow::KDevMainWindow( QWidget *parent, Qt::WFlags flags )
        : KMainWindow( parent, flags )
{
    setObjectName( QLatin1String( "KDevMainWindow" ) );
    d = new KDevMainWindowPrivate();
    d->center = new QStackedWidget( this );
    setCentralWidget( d->center );

    setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
}

KDevMainWindow::~ KDevMainWindow()
{}

void KDevMainWindow::setupActions()
{
    KStdAction::quit( this, SLOT( close() ), actionCollection() );

    KAction *action;

    QString app = qApp->applicationName();
    QString text = i18n( "Configure %1" ).arg( app );
    action = KStdAction::preferences( this, SLOT( settings() ),
                                      actionCollection(), "settings_configure" );
    action->setToolTip( text );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( text ).arg(
                              i18n( "Lets you customize %1." ).arg( app ) ) );

    action = new KAction( i18n( "Configure &Editor..." ), actionCollection(),
                          "settings_configure_editors" );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( configureEditors() ) );
    action->setToolTip( i18n( "Configure editor settings" ) );
    action->setWhatsThis( i18n( "<b>Configure editor</b><p>Opens editor configuration dialog." ) );
    action->setEnabled( false );

    action = KStdAction::showStatusbar( this, SLOT( toggleStatusbar() ),
                                        actionCollection(), "settings_show_statusbar" );
    action->setText( i18n( "Show &Statusbar" ) );
    action->setToolTip( i18n( "Show statusbar" ) );
    action->setWhatsThis( i18n( "<b>Show statusbar</b><p>Hides or shows the statusbar." ) );

    KToolBarPopupAction *popupAction;
    popupAction = new KToolBarPopupAction( KIcon( "stop" ),
                                           i18n( "&Stop" ),
                                           actionCollection(),
                                           "stop_processes" );

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

    //FIXME fix connection after gutting of KDevCore
    /*connect( KDevCore::getInstance(), SIGNAL( activeProcessChanged( KDevPlugin*, bool ) ),
    this, SLOT( activeProcessChanged( KDevPlugin*, bool ) ) );*/

    action = KStdAction::showMenubar(
                 this, SLOT( showMenuBar() ),
                 actionCollection(), "settings_show_menubar" );
    action->setToolTip( beautifyToolTip( action->text() ) );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( beautifyToolTip( action->text() ) ).arg( i18n( "Lets you toggle the menubar on/off." ) ) );

    action = KStdAction::keyBindings(
                 this, SLOT( keyBindings() ),
                 actionCollection(), "settings_configure_shortcuts" );
    action->setToolTip( beautifyToolTip( action->text() ) );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( beautifyToolTip( action->text() ) ).arg( i18n( "Lets you configure shortcut keys." ) ) );

    action = KStdAction::configureToolbars(
                 this, SLOT( configureToolbars() ),
                 actionCollection(), "settings_configure_toolbars" );
    action->setToolTip( beautifyToolTip( action->text() ) );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( beautifyToolTip( action->text() ) ).arg( i18n( "Lets you configure toolbars." ) ) );

    action = KStdAction::configureNotifications(
                 this, SLOT( configureNotifications() ),
                 actionCollection(), "settings_configure_notifications" );
    action->setToolTip( beautifyToolTip( action->text() ) );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( beautifyToolTip( action->text() ) ).arg( i18n( "Lets you configure system notifications." ) ) );

    action = new KAction( i18n( "&Next Window" ), actionCollection(), "view_next_window" );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoNextWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Right );
    action->setToolTip( i18n( "Next window" ) );
    action->setWhatsThis( i18n( "<b>Next window</b><p>Switches to the next window." ) );

    action = new KAction( i18n( "&Previous Window" ), actionCollection(), "view_previous_window" );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoPreviousWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Left );
    action->setToolTip( i18n( "Previous window" ) );
    action->setWhatsThis( i18n( "<b>Previous window</b><p>Switches to the previous window." ) );

    action = new KAction( i18n( "&Last Accessed Window" ), actionCollection(), "view_last_window" );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoLastWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Up );
    action->setToolTip( i18n( "Last accessed window" ) );
    action->setWhatsThis( i18n( "<b>Last accessed window</b><p>Switches to the last viewed window (Hold the Alt key pressed and walk on by repeating the Up key)." ) );

    action = new KAction( i18n( "&First Accessed Window" ), actionCollection(), "view_first_window" );
    connect( action, SIGNAL( triggered( bool ) ), SLOT( gotoFirstWindow() ) );
    action->setShortcut( Qt::ALT + Qt::Key_Down );
    action->setToolTip( i18n( "First accessed window" ) );
    action->setWhatsThis( i18n( "<b>First accessed window</b><p>Switches to the first accessed window (Hold the Alt key pressed and walk on by repeating the Down key)." ) );
}

void KDevMainWindow::init()
{
    setStandardToolBarMenuEnabled( true );
    setupActions();
    setStatusBar( new KDevStatusBar( this ) );

    createGUI( ShellExtension::getInstance() ->xmlFile() );

    connect( KDevCore::documentController(), SIGNAL( documentActivated( KDevDocument* ) ),
             this, SLOT( documentActivated( KDevDocument* ) ) );
    connect( KDevCore::projectController(), SIGNAL( projectOpened() ),
             this, SLOT( projectOpened() ) );
    connect( KDevCore::projectController(), SIGNAL( projectClosed() ),
             this, SLOT( projectClosed() ) );
}

void KDevMainWindow::fillContextMenu( KMenu *menu, const Context *context )
{
    //Perhaps we get rid of this framework and instead have every Context contains
    //a kactioncollection.  Plugins could add their actions directly to the context
    //object retrieved from KDevCore... ??
    emit contextMenu( menu, context );

    //Put this in every context menu so that plugins will be encouraged to allow shortcuts
    KAction * action = actionCollection() ->action( "settings_configure_shortcuts" );
    menu->addAction( action );
}

void KDevMainWindow::embedPartView( QWidget *view, const QString &title,
                                    const QString & /*toolTip*/ )
{
    if ( !view || title.isEmpty() )
        return ;

    d->center->addWidget( view );
}

void KDevMainWindow::embedSelectView( QWidget *view, const QString &title,
                                      const QString & /*toolTip*/ )
{
    if ( !view || title.isEmpty() )
        return ;

    QDockWidget * dock = new QDockWidget( title, this );
    dock->setObjectName( title );
    dock->setWidget( view );
    d->dockList.append( dock );
    addDockWidget( Qt::LeftDockWidgetArea, dock );
}

void KDevMainWindow::embedOutputView( QWidget *view, const QString &title,
                                      const QString & /*toolTip*/ )
{
    if ( !view || title.isEmpty() )
        return ;

    QDockWidget * dock = new QDockWidget( title, this );
    dock->setObjectName( title );
    dock->setWidget( view );
    d->dockList.append( dock );
    addDockWidget( Qt::BottomDockWidgetArea, dock );
}

void KDevMainWindow::embedSelectViewRight( QWidget *view,
        const QString &title,
        const QString & /*toolTip*/ )
{
    if ( !view || title.isEmpty() )
        return ;

    QDockWidget * dock = new QDockWidget( title, this );
    dock->setObjectName( title );
    dock->setWidget( view );
    d->dockList.append( dock );
    addDockWidget( Qt::RightDockWidgetArea, dock );
}

void KDevMainWindow::removeView( QWidget *view )
{
    if ( !view )
        return ;

    foreach( QDockWidget * dock, d->dockList )
    {
        if ( dock->widget() == view )
        {
            removeDockWidget( dock );
            d->dockList.removeAll( dock );
            delete dock;
            break;
        }
    }

    d->center->removeWidget( view );
}

void KDevMainWindow::setViewAvailable( QWidget *pView, bool bEnabled )
{
    Q_UNUSED( pView );
    Q_UNUSED( bEnabled );
    //TODO hide docs
}

bool KDevMainWindow::containsWidget( QWidget *widget ) const
{
    return ( d->center->indexOf( widget ) != -1 );
}

void KDevMainWindow::setCurrentWidget( QWidget *widget )
{
    if ( !widget )
        return ;

    d->center->setCurrentWidget( widget );
}

void KDevMainWindow::raiseView( QWidget *view, Qt::DockWidgetArea area )
{
    if ( !view )
        return ;

    foreach( QDockWidget * dock, d->dockList )
    {
        if ( dock->widget() == view )
        {
            addDockWidget( area, dock );
            dock->show();
            break;
        }
    }
}

void KDevMainWindow::lowerView( QWidget * view )
{
    if ( !view )
        return ;

    foreach( QDockWidget * dock, d->dockList )
    {
        if ( dock->widget() == view )
        {
            removeDockWidget( dock );
            break;
        }
    }
}

void KDevMainWindow::loadSettings()
{
    KConfig * config = KDevConfig::standard();

    applyMainWindowSettings( config, QLatin1String( "KDevMainWindow" ) );
}

void KDevMainWindow::saveSettings()
{
    KConfig * config = KDevConfig::standard();

    saveMainWindowSettings( config, QLatin1String( "KDevMainWindow" ) );
}

void KDevMainWindow::setVisible( bool visible )
{
    loadSettings();

    KMainWindow::setVisible( visible );

    emit finishedLoading();
}

void KDevMainWindow::gotoNextWindow()
{
    if ( ( d->center->currentIndex() + 1 ) < d->center->count() )
        d->center->setCurrentIndex( d->center->currentIndex() + 1 );
    else
        d->center->setCurrentIndex( 0 );
}

void KDevMainWindow::gotoPreviousWindow()
{
    if ( ( d->center->currentIndex() - 1 ) >= 0 )
        d->center->setCurrentIndex( d->center->currentIndex() - 1 );
    else
        d->center->setCurrentIndex( d->center->count() - 1 );
}

void KDevMainWindow::gotoFirstWindow()
{
    d->center->setCurrentIndex( 0 );
}

void KDevMainWindow::gotoLastWindow()
{
    d->center->setCurrentIndex( d->center->count() - 1 );
}

void KDevMainWindow::projectOpened()
{
    QString app = i18n( "Project" );
    QString text = i18n( "Configure %1" ).arg( app );
    KAction *action = actionCollection() ->action( "settings_configure" );
    action->setToolTip( text );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( text ).arg(
                              i18n( "Lets you customize %1." ).arg( app ) ) );
}

void KDevMainWindow::projectClosed()
{
    QString app = qApp->applicationName();
    QString text = i18n( "Configure %1" ).arg( app );
    KAction *action = actionCollection() ->action( "settings_configure" );
    action->setToolTip( text );
    action->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( text ).arg(
                              i18n( "Lets you customize %1." ).arg( app ) ) );
}

void KDevMainWindow::configureToolbars()
{}

void KDevMainWindow::newToolbarConfig()
{
    applyMainWindowSettings( KGlobal::config(),
                             QLatin1String( "KDevMainWindow" ) );
}

bool KDevMainWindow::queryClose()
{
    //All KDevCore API objects must release all resources which
    //depend upon one another.
    KDevCore::projectController() ->cleanUp();
    KDevCore::documentController() ->cleanUp();
    KDevCore::pluginController() ->cleanUp();

    saveSettings();
    return true;
}

void KDevMainWindow::setupWindowMenu()
{
    //FIXME This should setup a window menu or perhaps dialog instead.
    // Either way, we need one with a scroll bar.  I'm tired of menus that take
    // up the entire screen.
}

void KDevMainWindow::fillWindowMenu()
{
    //FIXME This should fill a window menu or perhaps dialog instead.
    // Either way, we need one with a scroll bar.  I'm tired of menus that take
    // up the entire screen.
}

QString KDevMainWindow::beautifyToolTip( const QString& text ) const
{
    QString temp = text;
    temp.replace( QRegExp( "&" ), "" );
    temp.replace( QRegExp( "\\.\\.\\." ), "" );
    return temp;
}

void KDevMainWindow::reportBug()
{}

void KDevMainWindow::toggleStatusbar()
{
    KToggleAction * action =
            qobject_cast< KToggleAction*>( actionCollection() ->action( "settings_show_statusbar" ) );
    statusBar() ->setHidden( !action->isChecked() );
}

void KDevMainWindow::stopButtonPressed()
{}

void KDevMainWindow::activeProcessChanged( KDevPlugin* plugin, bool active )
{
    Q_UNUSED( plugin );
    Q_UNUSED( active );
}

void KDevMainWindow::stopPopupActivated( int id )
{
    Q_UNUSED( id );
}

void KDevMainWindow::stopMenuAboutToShow()
{}

void KDevMainWindow::showMenuBar()
{}

void KDevMainWindow::configureNotifications()
{
    KNotifyDialog::configure( this, "Notification Configuration Dialog" );
}

void KDevMainWindow::settings()
{
    if ( !d->settingsDialog )
        d->settingsDialog = new KSettings::Dialog(QStringList("kdevelop"),
                                KSettings::Dialog::Static, this );
    d->settingsDialog->show();
}

void KDevMainWindow::configureEditors()
{
    //FIXME Change this so that it is embedded in our config dialog.
    //Perhaps this will require a change to the KTextEditor interface too...
    KTextEditor::Document * doc =
        KDevCore::documentController() ->activeDocument() ->textDocument();
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
    editor->configDialog( this );
}

void KDevMainWindow::keyBindings()
{}

void KDevMainWindow::documentActivated( KDevDocument *document )
{
    KAction * action = actionCollection() ->action( "settings_configure_editors" );
    action->setEnabled( document->textDocument() );
}

#include "kdevmainwindow.moc"

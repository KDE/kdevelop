/* This file is part of the KDE project
  Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
  Copyright (C) 2003 John Firebaugh <jfirebaugh@kde.org>

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
#include <kicon.h>
#include <kaction.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kstdaction.h>
#include <kparts/part.h>
#include <kmessagebox.h>
#include <kedittoolbar.h>
#include <kapplication.h>
#include <kactioncollection.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>

#include "kdevconfig.h"
#include "kdevprofile.h"
#include "kdevprofileengine.h"

#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "statusbar.h"
#include "shellextension.h"
#include "mainwindowshare.h"
#include "kdevplugincontroller.h"
#include "kdevprojectcontroller.h"
#include "kdevdocumentcontroller.h"

KDevMainWindow::KDevMainWindow( QWidget *parent, Qt::WFlags flags )
        : KMainWindow( parent, flags )
{
    setObjectName( QLatin1String( "KDevMainWindow" ) );
    m_mainWindowShare = new MainWindowShare( this );
    m_center = new QStackedWidget( this );
    setCentralWidget( m_center );

    setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
}

KDevMainWindow::~ KDevMainWindow()
{}

void KDevMainWindow::init()
{
    setStandardToolBarMenuEnabled( true );

    KStdAction::configureToolbars( this, SLOT( configureToolbars() ),
                                   actionCollection(),
                                   "set_configure_toolbars" );

    m_mainWindowShare->createActions();

    connect( m_mainWindowShare, SIGNAL( gotoNextWindow() ),
             this, SLOT( gotoNextWindow() ) );
    connect( m_mainWindowShare, SIGNAL( gotoPreviousWindow() ),
             this, SLOT( gotoPreviousWindow() ) );
    connect( m_mainWindowShare, SIGNAL( gotoFirstWindow() ),
             this, SLOT( gotoFirstWindow() ) );
    connect( m_mainWindowShare, SIGNAL( gotoLastWindow() ),
             this, SLOT( gotoLastWindow() ) );

    new KDevStatusBar( this );

    createGUI( ShellExtension::getInstance() ->xmlFile() );

    m_mainWindowShare->init();
    setupWindowMenu();

    connect( KDevCore::projectController(), SIGNAL( projectOpened() ),
             this, SLOT( projectOpened() ) );
}

void KDevMainWindow::embedPartView( QWidget *view, const QString &title,
                                    const QString & /*toolTip*/ )
{
    if ( !view || title.isEmpty() )
        return ;

    m_center->addWidget( view );
}

void KDevMainWindow::embedSelectView( QWidget *view, const QString &title,
                                      const QString & /*toolTip*/ )
{
    if ( !view || title.isEmpty() )
        return ;

    QDockWidget * dock = new QDockWidget( title, this );
    dock->setObjectName( title );
    dock->setWidget( view );
    m_dockList.append( dock );
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
    m_dockList.append( dock );
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
    m_dockList.append( dock );
    addDockWidget( Qt::RightDockWidgetArea, dock );
}

void KDevMainWindow::removeView( QWidget *view )
{
    if ( !view )
        return ;

    foreach( QDockWidget * dock, m_dockList )
    {
        if ( dock->widget() == view )
        {
            removeDockWidget( dock );
            m_dockList.removeAll( dock );
            delete dock;
            break;
        }
    }

    m_center->removeWidget( view );
}

void KDevMainWindow::setViewAvailable( QWidget *pView, bool bEnabled )
{
    Q_UNUSED( pView );
    Q_UNUSED( bEnabled );
    //TODO hide docs
}

bool KDevMainWindow::containsWidget( QWidget *widget ) const
{
    return ( m_center->indexOf( widget ) != -1 );
}

void KDevMainWindow::setCurrentWidget( QWidget *widget )
{
    if ( !widget )
        return ;

    m_center->setCurrentWidget( widget );
}

void KDevMainWindow::raiseView( QWidget *view, Qt::DockWidgetArea area )
{
    if ( !view )
        return ;

    foreach( QDockWidget * dock, m_dockList )
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

    foreach( QDockWidget * dock, m_dockList )
    {
        if ( dock->widget() == view )
        {
            removeDockWidget( dock );
            break;
        }
    }
}

KStatusBar *KDevMainWindow::statusBar()
{
    return statusBar();
}

void KDevMainWindow::loadSettings()
{
    KConfig * config = KDevConfig::standard();

    applyMainWindowSettings( config, QLatin1String( "KDevMainWindow" ) );

    show(); //kind of crucial

    emit finishedLoading();
}

void KDevMainWindow::saveSettings( )
{
    KConfig * config = KDevConfig::standard();

    saveMainWindowSettings( config, QLatin1String( "KDevMainWindow" ) );
}

void KDevMainWindow::gotoNextWindow()
{
    if ( ( m_center->currentIndex() + 1 ) < m_center->count() )
        m_center->setCurrentIndex( m_center->currentIndex() + 1 );
    else
        m_center->setCurrentIndex( 0 );
}

void KDevMainWindow::gotoPreviousWindow()
{
    if ( ( m_center->currentIndex() - 1 ) >= 0 )
        m_center->setCurrentIndex( m_center->currentIndex() - 1 );
    else
        m_center->setCurrentIndex( m_center->count() - 1 );
}

void KDevMainWindow::gotoFirstWindow()
{
    m_center->setCurrentIndex( 0 );
}

void KDevMainWindow::gotoLastWindow()
{
    m_center->setCurrentIndex( m_center->count() - 1 );
}

void KDevMainWindow::projectOpened()
{
    setCaption( QString() );
}

void KDevMainWindow::configureToolbars()
{
    saveMainWindowSettings( KGlobal::config(),
                            QLatin1String( "KDevMainWindow" ) );
    KEditToolbar* dlg = new KEditToolbar( factory() );
    connect( dlg, SIGNAL( newToolbarConfig() ),
             this, SLOT( newToolbarConfig() ) );
    connect( dlg, SIGNAL( finished( int ) ),
             dlg, SLOT( deleteLater() ) );
    dlg->show();
}

void KDevMainWindow::newToolbarConfig()
{
    setupWindowMenu();
    applyMainWindowSettings( KGlobal::config(),
                             QLatin1String( "KDevMainWindow" ) );
}

bool KDevMainWindow::queryClose()
{
    bool success = true;
    //FIXME change all of these method calls to cleanUp()
    if ( !KDevCore::projectController() ->closeProject() )
        success = false;

    if ( !KDevCore::documentController() ->querySaveDocuments() )
        success = false;

    if ( !KDevCore::documentController() ->readyToClose() )
        success = false;

    if ( !KDevCore::pluginController() ->unloadPlugins() )
        success = false;

    saveSettings();
    return success;
}

bool KDevMainWindow::queryExit()
{
    return true;
}

void KDevMainWindow::setupWindowMenu()
{
    /*    get the xmlgui created one instead*/
    m_windowMenu = qFindChild<KMenu *>( this, QLatin1String( "window" ) );

    m_windowMenu->addAction( actionCollection() ->action( "file_close" ) );
    m_windowMenu->addAction( actionCollection() ->action( "file_close_all" ) );
    m_windowMenu->addAction( actionCollection() ->action( "file_closeother" ) );

    QObject::connect( m_windowMenu, SIGNAL( activated( int ) ),
                      this, SLOT( openURL( int ) ) );
    QObject::connect( m_windowMenu, SIGNAL( aboutToShow() ),
                      this, SLOT( fillWindowMenu() ) );
}

void KDevMainWindow::openURL( int w )
{
    foreach( WinInfo pair, m_windowList )
    {
        if ( pair.first == w )
        {
            if ( pair.second )
            {
                pair.second->activate();
                return ;
            }
        }
    }
}

void KDevMainWindow::fillWindowMenu()
{
    // clear menu
    foreach( WinInfo pair, m_windowList )
    {
        m_windowMenu->removeItem( pair.first );
    }

    QMap<QString, KDevDocument*> map;
    QStringList string_list;
    foreach ( KDevDocument * file, KDevCore::documentController() ->openDocuments() )
    {
        map[ file->url().fileName() ] = file;
        string_list.append( file->url().fileName() );
    }
    string_list.sort();

    QList<KDevDocument*> list;
    for ( int i = 0; i != string_list.size(); ++i )
        list.append( map[ string_list[ i ] ] );

    if ( list.count() > 0 )
        m_windowList << qMakePair( m_windowMenu->insertSeparator(), static_cast<KDevDocument*>( 0L ) );

    int i = 0;
    foreach ( KDevDocument * file, list )
    {
        int temp = m_windowMenu->insertItem( i < 10 ? QString( "&%1 %2" ).arg( i ).arg( file->url().fileName() ) : file->url().fileName() );
        m_windowList << qMakePair( temp, file );
        ++i;
    }
}

#include "kdevmainwindow.moc"

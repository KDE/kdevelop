/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "simplemainwindow.h"

#include <qdockwidget.h>
#include <qstackedwidget.h>

#include <kmenu.h>
#include <kicon.h>
#include <kglobal.h>
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

#include <kdevprofile.h>
#include <kdevprofileengine.h>

#include "core.h"
#include "toplevel.h"
#include "statusbar.h"
#include "editorproxy.h"
#include "projectmanager.h"
#include "shellextension.h"
#include "mainwindowshare.h"
#include "plugincontroller.h"
#include "documentationpart.h"
#include "documentcontroller.h"

SimpleMainWindow::SimpleMainWindow( QWidget *parent, Qt::WFlags flags )
        : KMainWindow( parent, flags )
{
    resize( 800, 600 );
    m_mainWindowShare = new MainWindowShare( this );

    m_center = new QStackedWidget( this );
    setCentralWidget( m_center );

    setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
}

SimpleMainWindow::~ SimpleMainWindow( )
{
    TopLevel::invalidateInstance( this );
}

void SimpleMainWindow::init()
{
    setStandardToolBarMenuEnabled( true );

    DocumentController::createInstance( this );

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
    menuBar() ->setEnabled( false );

    //FIXME: this checks only for global offers which is not quite correct
    //because a profile can offer core plugins and no global plugins.
    if ( PluginController::getInstance() ->engine().allOffers( ProfileEngine::Global ).isEmpty() )
    {
        KMessageBox::sorry( this,
                            i18n( "Could Not Find Plugins" ),
                            i18n( "Could Not Find Plugins" ) );
    }

    connect( Core::getInstance(), SIGNAL( coreInitialized() ),
             this, SLOT( coreInitialized() ) );
    connect( Core::getInstance(), SIGNAL( projectOpened() ),
             this, SLOT( projectOpened() ) );

    loadSettings();
}

void SimpleMainWindow::embedPartView( QWidget *view, const QString &title,
                                      const QString & /*toolTip*/ )
{
    if ( !view || title.isEmpty() )
        return ;

    m_center->addWidget( view );
}

void SimpleMainWindow::embedSelectView( QWidget *view, const QString &title,
                                        const QString & /*toolTip*/ )
{
    if ( !view )
        return ;

    QDockWidget * dock = new QDockWidget( title, this );
    dock->setWidget( view );
    m_dockList.append( dock );
    addDockWidget( Qt::LeftDockWidgetArea, dock );
}

void SimpleMainWindow::embedOutputView( QWidget *view, const QString &title,
                                        const QString & /*toolTip*/ )
{
    if ( !view )
        return ;

    QDockWidget * dock = new QDockWidget( title, this );
    dock->setWidget( view );
    m_dockList.append( dock );
    addDockWidget( Qt::BottomDockWidgetArea, dock );
}

void SimpleMainWindow::embedSelectViewRight( QWidget *view,
        const QString &title,
        const QString & /*toolTip*/ )
{
    if ( !view )
        return ;

    QDockWidget * dock = new QDockWidget( title, this );
    dock->setWidget( view );
    m_dockList.append( dock );
    addDockWidget( Qt::RightDockWidgetArea, dock );
}

void SimpleMainWindow::removeView( QWidget *view )
{
    if ( !view )
        return ;

    foreach( QDockWidget *dock, m_dockList )
    {
        if ( dock->widget() == view )
        {
            removeDockWidget( dock );
            m_dockList.remove( dock );
            delete dock;
            break;
        }
    }

    m_center->removeWidget( view );
}

void SimpleMainWindow::setViewAvailable( QWidget *pView, bool bEnabled )
{
    //TODO hide docs
}

void SimpleMainWindow::setCurrentWidget( QWidget * widget  )
{
    if ( !widget )
        return ;
    m_center->setCurrentWidget( widget );
}

void SimpleMainWindow::raiseView( QWidget * view, Qt::DockWidgetArea area )
{
    if ( !view )
        return ;

    foreach( QDockWidget *dock, m_dockList )
    {
        if ( dock->widget() == view )
        {
            addDockWidget( area, dock );
            dock->show();
            break;
        }
    }
}

void SimpleMainWindow::lowerView( QWidget * view )
{
    if ( !view )
        return ;

    foreach( QDockWidget *dock, m_dockList )
    {
        if ( dock->widget() == view )
        {
            removeDockWidget( dock );
            break;
        }
    }
}

void SimpleMainWindow::loadSettings()
{
    KConfig * config = KGlobal::config();

    ProjectManager::getInstance() ->loadSettings();
    applyMainWindowSettings( config, QLatin1String( "SimpleMainWindow" ) );
}

void SimpleMainWindow::saveSettings( )
{
    KConfig * config = KGlobal::config();

    ProjectManager::getInstance() ->saveSettings();
    saveMainWindowSettings( config, QLatin1String( "SimpleMainWindow" ) );
}

KMainWindow *SimpleMainWindow::main()
{
    return this;
}

void SimpleMainWindow::gotoNextWindow()
{
    if ( ( m_center->currentIndex() + 1 ) < m_center->count() )
        m_center->setCurrentIndex( m_center->currentIndex() + 1 );
    else
        m_center->setCurrentIndex( 0 );
}

void SimpleMainWindow::gotoPreviousWindow()
{
    if ( ( m_center->currentIndex() - 1 ) >= 0 )
        m_center->setCurrentIndex( m_center->currentIndex() - 1 );
    else
        m_center->setCurrentIndex( m_center->count() - 1 );
}

void SimpleMainWindow::gotoFirstWindow()
{
    m_center->setCurrentIndex( 0 );
}

void SimpleMainWindow::gotoLastWindow()
{
    m_center->setCurrentIndex( m_center->count() - 1 );
}

void SimpleMainWindow::coreInitialized()
{
    menuBar() ->setEnabled( true );
}

void SimpleMainWindow::projectOpened()
{
    setCaption( QString() );
}

void SimpleMainWindow::configureToolbars()
{
    saveMainWindowSettings( KGlobal::config(),
                            QLatin1String( "SimpleMainWindow" ) );
    KEditToolbar* dlg = new KEditToolbar( factory() );
    connect( dlg, SIGNAL( newToolbarConfig() ),
             this, SLOT( newToolbarConfig() ) );
    connect( dlg, SIGNAL( finished( int ) ),
             dlg, SLOT( deleteLater() ) );
    dlg->show();
}

void SimpleMainWindow::newToolbarConfig()
{
    setupWindowMenu();
    //FIXME
//     m_mainWindowShare->slotGUICreated(
//         DocumentController::getInstance() ->activePart() );
    applyMainWindowSettings( KGlobal::config(),
                             QLatin1String( "SimpleMainWindow" ) );
}

bool SimpleMainWindow::queryClose()
{
    saveSettings();
    return Core::getInstance() ->queryClose();
}

bool SimpleMainWindow::queryExit()
{
    return true;
}

void SimpleMainWindow::setupWindowMenu()
{
    /*    get the xmlgui created one instead*/
    m_windowMenu = qFindChild<KMenu *>( main(), QLatin1String( "window" ) );

    m_windowMenu->addAction( actionCollection() ->action( "file_close" ) );
    m_windowMenu->addAction( actionCollection() ->action( "file_close_all" ) );
    m_windowMenu->addAction( actionCollection() ->action( "file_closeother" ) );

    QObject::connect( m_windowMenu, SIGNAL( activated( int ) ),
                      this, SLOT( openURL( int ) ) );
    QObject::connect( m_windowMenu, SIGNAL( aboutToShow() ),
                      this, SLOT( fillWindowMenu() ) );
}

void SimpleMainWindow::openURL( int w )
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

void SimpleMainWindow::fillWindowMenu()
{
    // clear menu
    foreach( WinInfo pair, m_windowList )
    {
        m_windowMenu->removeItem( pair.first );
    }

    QMap<QString, KDevDocument*> map;
    QStringList string_list;
    foreach ( KDevDocument * file, DocumentController::getInstance() ->openDocuments() )
    {
        map[ file->url().fileName() ] = file;
        string_list.append( file->url().fileName() );
    }
    string_list.sort();

    QList<KDevDocument*> list;
    for ( uint i = 0; i != string_list.size(); ++i )
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

#include "simplemainwindow.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

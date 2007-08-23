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
#include "mainwindow.h"
#include "mainwindow_p.h"

#include <QtCore/QHash>
#include <QtGui/QDockWidget>
#include <QtGui/QStackedWidget>

#include <kmenu.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kactioncollection.h>

#include "shellextension.h"
#include "partcontroller.h"
#include "plugincontroller.h"

namespace KDevelop
{

MainWindow::MainWindow( Sublime::Controller *parent, Qt::WFlags flags )
        : Sublime::MainWindow( parent, flags )
{
    KConfigGroup cg = KGlobal::config()->group( "UiSettings" );
    int bottomleft = cg.readEntry( "BottomLeftCornerOwner", 0 );
    int bottomright = cg.readEntry( "BottomRightCornerOwner", 0 );
    kDebug(9501) << "Bottom Left:" << bottomleft;
    kDebug(9501) << "Bottom Right:" << bottomright;

    // 0 means vertical dock (left, right), 1 means horizontal dock( top, bottom )
    if( bottomleft == 0 )
        setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    else if( bottomleft == 1 )
        setCorner( Qt::BottomLeftCorner, Qt::BottomDockWidgetArea );

    if( bottomright == 0 )
        setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
    else if( bottomright == 1 )
        setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );

    setObjectName( "MainWindow" );
    d = new MainWindowPrivate(this);

    setStandardToolBarMenuEnabled( true );
    d->setupActions();
//     setStatusBar( new KDevelop::StatusBar( this ) );

    setXMLFile( ShellExtension::getInstance() ->xmlFile() );
}

MainWindow::~ MainWindow()
{
    delete d;
}

// TODO why doesn't called automatically when the settings are changed??
void MainWindow::loadSettings()
{
    kDebug(9501) << "Loading Settings";
    KConfigGroup cg = KGlobal::config()->group( "UiSettings" );
    bool verticaltabs = cg.readEntry( "Use Vertical Tabs", true );
    bool verticaltitle = cg.readEntry( "Use Vertical TitleBar", true );

    kDebug(9501) << verticaltabs <<"|" << verticaltitle;
    if( verticaltabs )
        setVerticalToolViewTabsMode( Sublime::MainWindow::UseVerticalTabs );
    else
        setVerticalToolViewTabsMode( Sublime::MainWindow::NoVerticalTabs );
    if( verticaltitle )
        setVerticalToolViewTitleBarMode( Sublime::MainWindow::HorizontalDocks );
    else
        setVerticalToolViewTitleBarMode( Sublime::MainWindow::NoDocks );

    // dock widget corner layout
    int bottomleft = cg.readEntry( "BottomLeftCornerOwner", 0 );
    int bottomright = cg.readEntry( "BottomRightCornerOwner", 0 );
    kDebug(9501) << "Bottom Left:" << bottomleft;
    kDebug(9501) << "Bottom Right:" << bottomright;

    // 0 means vertical dock (left, right), 1 means horizontal dock( top, bottom )
    if( bottomleft == 0 )
        setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    else if( bottomleft == 1 )
        setCorner( Qt::BottomLeftCorner, Qt::BottomDockWidgetArea );

    if( bottomright == 0 )
        setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );
    else if( bottomright == 1 )
        setCorner( Qt::BottomRightCorner, Qt::BottomDockWidgetArea );

    Sublime::MainWindow::loadSettings();

}

void MainWindow::saveSettings()
{
    Sublime::MainWindow::saveSettings();
}

void MainWindow::initialize()
{
    createGUI(0);
    Core::self()->partManager()->addManagedTopLevelWidget(this);
    connect( Core::self()->pluginController(), SIGNAL(pluginLoaded(IPlugin*)),
             d, SLOT(addPlugin(IPlugin*)));
    connect( Core::self()->partManager(), SIGNAL(activePartChanged(KParts::Part*)),
        d, SLOT(activePartChanged(KParts::Part*)));
    connect( this, SIGNAL(activeViewChanged(Sublime::View*)),
        d, SLOT(changeActiveView(Sublime::View*)));
/*    connect( Core::documentController(), SIGNAL( documentActivated( Document* ) ),
             d, SLOT( documentActivated( Document* ) ) );
    connect( Core::projectController(), SIGNAL( projectOpened() ),
             d, SLOT( projectOpened() ) );
    connect( Core::projectController(), SIGNAL( projectClosed() ),
             d, SLOT( projectClosed() ) );*/
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

void MainWindow::setVisible( bool visible )
{
    KXmlGuiWindow::setVisible( visible );
    emit finishedLoading();
}

bool MainWindow::queryClose()
{
    return Sublime::MainWindow::queryClose();
}

}

#include "mainwindow.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

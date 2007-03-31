/* This file is part of the KDevelop project
Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
Copyright (C) 2003 John Firebaugh <jfirebaugh@kde.org>
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2006, 2007 Alexander Dymo <adymo@kdevelop.org>

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

#include <kmenu.h>
#include <kglobal.h>
#include <klocale.h>
#include <ktemporaryfile.h>
#include <kactioncollection.h>

#include "shellextension.h"
#include "partcontroller.h"
#include "mainwindow_p.h"
#include "plugincontroller.h"

namespace KDevelop
{

MainWindow::MainWindow( Sublime::Controller *parent, Qt::WFlags flags )
        : Sublime::MainWindow( parent, flags )
{
    setObjectName( "MainWindow" );
    d = new MainWindowPrivate(this);
    d->center = new QStackedWidget( this );
    setCentralWidget( d->center );

    setCorner( Qt::TopLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::TopRightCorner, Qt::RightDockWidgetArea );
    setCorner( Qt::BottomLeftCorner, Qt::LeftDockWidgetArea );
    setCorner( Qt::BottomRightCorner, Qt::RightDockWidgetArea );

    setStandardToolBarMenuEnabled( true );
    d->setupActions();
//     setStatusBar( new KDevelop::StatusBar( this ) );

    setXMLFile( ShellExtension::getInstance() ->xmlFile() );
}

MainWindow::~ MainWindow()
{
    delete d;
}

void MainWindow::loadSettings()
{
    kDebug(9000) << "Loading Settings" << endl;
    KConfigGroup cg = KGlobal::config()->group( "UiSettings" );
    bool verticaltabs = cg.readEntry( "Use Vertical Tabs", true );
    bool verticaltitle = cg.readEntry( "Use Vertical TitleBar", true );

    kDebug(9000) << verticaltabs <<"|" << verticaltitle << endl;
    if( verticaltabs )
        setVerticalToolViewTabsMode( Sublime::MainWindow::UseVerticalTabs );
    else
        setVerticalToolViewTabsMode( Sublime::MainWindow::NoVerticalTabs );
    if( verticaltitle )
        setVerticalToolViewTitleBarMode( Sublime::MainWindow::HorizontalDocks );
    else
        setVerticalToolViewTitleBarMode( Sublime::MainWindow::NoDocks );
    Sublime::MainWindow::loadSettings();

}

void MainWindow::saveSettings()
{
    Sublime::MainWindow::saveSettings();
}

void MainWindow::initialize()
{
    createGUI(0);
    Core::self()->partController()->addManagedTopLevelWidget(this);
    connect( Core::self()->pluginController(), SIGNAL(pluginLoaded(IPlugin*)),
             d, SLOT(addPlugin(IPlugin*)));
    connect( Core::self()->partController(), SIGNAL(activePartChanged(KParts::Part*)),
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
    KMainWindow::setVisible( visible );
    emit finishedLoading();
}

bool MainWindow::queryClose()
{
    return Sublime::MainWindow::queryClose();
}

}

#include "mainwindow.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

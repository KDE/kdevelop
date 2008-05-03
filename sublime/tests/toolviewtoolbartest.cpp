/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
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

#include "toolviewtoolbartest.h"

#include <QtTest/QtTest>

#include <QListView>
#include <QTextEdit>
#include <QDockWidget>
#include <QToolBar>

#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>
#include <sublime/mainwindow.h>
#include <sublime/tooldocument.h>

#include <tests/common/kdevtest.h>
#include "kdebug.h"

using namespace Sublime;

class SpecialWidgetToolBar1Factory : public SimpleToolWidgetFactory<QTextEdit> {
public:
    SpecialWidgetToolBar1Factory(const QString &id): SimpleToolWidgetFactory<QTextEdit>(id) {}
    virtual QList<QAction*> toolBarActions( QWidget* ) const
    {
        QList<QAction*> actions;
        actions << new QAction( "Go Up", 0 );
        actions << new QAction( "Go Down", 0 );
        actions << new QAction( "Go Right", 0 );
        actions << new QAction( "Go Left", 0 );
        actions << new QAction( "Run Up", 0 );
        actions << new QAction( "Run Down", 0 );
        actions << new QAction( "Run Right", 0 );
        actions << new QAction( "Run Left", 0 );
        return actions;
    }
};

class SpecialWidgetToolBar2Factory : public SimpleToolWidgetFactory<QTextEdit> {
public:
    SpecialWidgetToolBar2Factory(const QString &id): SimpleToolWidgetFactory<QTextEdit>(id) {}
    virtual QList<QAction*> toolBarActions( QWidget* ) const
    {
        QList<QAction*> actions;
        actions << new QAction( "Go Up", 0 );
        actions << new QAction( "Go Down", 0 );
        actions << new QAction( "Go Right", 0 );
        actions << new QAction( "Go Left", 0 );
        actions << new QAction( "Jump Up", 0 );
        actions << new QAction( "Jump Down", 0 );
        actions << new QAction( "Jump Right", 0 );
        actions << new QAction( "Jump Left", 0 );
        return actions;
    }
};


void ToolViewToolBarTest::init()
{
    controller = new Controller(this);
    tool1 = new ToolDocument( "tool1", controller, new SpecialWidgetToolBar1Factory("tool1") );
    tool2 = new ToolDocument( "tool2", controller, new SpecialWidgetToolBar2Factory("tool2") );

    area = new Area( controller, "Area" );
    viewT11 = tool1->createView();
    area->addToolView( viewT11, Sublime::Bottom );
    viewT21 = tool2->createView();
    area->addToolView( viewT21, Sublime::Left );
}

void ToolViewToolBarTest::cleanup()
{
    delete controller;
}

ToolViewToolBarTest::ToolViewToolBarTest()
{
}

ToolViewToolBarTest::~ToolViewToolBarTest()
{
}


void ToolViewToolBarTest::testToolBarExistence()
{
    MainWindow mw(controller);

    controller->showArea(area, &mw);

    QList<QDockWidget*> dockWidgets = mw.findChildren<QDockWidget*>();
    foreach( QDockWidget* dock, dockWidgets )
    {
        QList<QToolBar*> toolbars = dock->titleBarWidget()->findChildren<QToolBar*>();
        QCOMPARE( toolbars.count(), 1 );
        QToolBar* toolbar = toolbars.at(0);
        QVERIFY( !toolbar->isFloatable() );
        QVERIFY( !toolbar->isMovable() );
        QCOMPARE( toolbar->iconSize(), QSize( 16, 16 ) );
        QList<QAction*> actions = toolbar->actions();
        QCOMPARE( actions.count(), 8 );
        QCOMPARE( actions.at(0)->text(), QString( "Go Up" ) );
        QCOMPARE( actions.at(1)->text(), QString( "Go Down" ) );
        QCOMPARE( actions.at(2)->text(), QString( "Go Right" ) );
        QCOMPARE( actions.at(3)->text(), QString( "Go Left" ) );
        if( actions.at(4)->text().startsWith("Jump") )
        {
            QCOMPARE( toolbar->orientation(), Qt::Horizontal );
            QCOMPARE( actions.at(4)->text(), QString( "Jump Up" ) );
            QCOMPARE( actions.at(5)->text(), QString( "Jump Down" ) );
            QCOMPARE( actions.at(6)->text(), QString( "Jump Right" ) );
            QCOMPARE( actions.at(7)->text(), QString( "Jump Left" ) );
        } else
        {
            QCOMPARE( toolbar->orientation(), Qt::Vertical );
            QCOMPARE( actions.at(4)->text(), QString( "Run Up" ) );
            QCOMPARE( actions.at(5)->text(), QString( "Run Down" ) );
            QCOMPARE( actions.at(6)->text(), QString( "Run Right" ) );
            QCOMPARE( actions.at(7)->text(), QString( "Run Left" ) );
        }
    }
}

void ToolViewToolBarTest::testToolViewMove()
{
    MainWindow mw(controller);

    controller->showArea(area, &mw);

    area->moveToolView( viewT11, Sublime::Right );
    area->moveToolView( viewT21, Sublime::Bottom );

    QList<QDockWidget*> dockWidgets = mw.findChildren<QDockWidget*>();
    foreach( QDockWidget* dock, dockWidgets )
    {
        QList<QToolBar*> toolbars = dock->titleBarWidget()->findChildren<QToolBar*>();
        QCOMPARE( toolbars.count(), 1 );
        QToolBar* toolbar = toolbars.at(0);
        foreach( QAction* act, toolbar->actions() )
        {
            if( act->text().startsWith( "Jump" ) )
            {
                QCOMPARE( toolbar->orientation(), Qt::Vertical );
                break;
            } else if( act->text().startsWith( "Run" ) )
            {
                QCOMPARE( toolbar->orientation(), Qt::Horizontal );
                break;
            }
        }
    }
}

KDEVTEST_MAIN(ToolViewToolBarTest)
#include "toolviewtoolbartest.moc"

/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>                 *
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

#include "test_toolviewtoolbar.h"

#include <QtTest/QtTest>

#include <QListView>
#include <QTextEdit>
#include <QDockWidget>
#include <QToolBar>

#include <sublime/idealcontroller.h>
#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>
#include <sublime/mainwindow.h>
#include <sublime/tooldocument.h>

#include "kdebug.h"

using namespace Sublime;

class ToolViewToolBarFactory : public SimpleToolWidgetFactory<QTextEdit> {
public:
    ToolViewToolBarFactory(const QString &id): SimpleToolWidgetFactory<QTextEdit>(id) {}
    virtual QList<QAction*> toolBarActions( QWidget* ) const
    {
        QAction* action = new QAction(actionText, 0);
        return QList<QAction*>() << action;
    }
    QString actionText;
};

void TestToolViewToolBar::init()
{
    // this is starting to become a GeneralFixture
    controller = new Controller(this);
    area = new Area( controller, "Area" );
    MainWindow* mw = new MainWindow(controller);

    // a horizontal tool with toolbar
    ToolViewToolBarFactory* factoryT1 = new ToolViewToolBarFactory("tool1factory");
    actionTextT1 = "Tool1Action";
    factoryT1->actionText = actionTextT1;
    tool1 = new ToolDocument( "tool1", controller, factoryT1 );
    viewT11 = tool1->createView();
    area->addToolView( viewT11, Sublime::Bottom );

    // a vertical tool with toolbar
    ToolViewToolBarFactory* factoryT2 = new ToolViewToolBarFactory("tool2factory");
    actionTextT2 = "Tool2Action";
    factoryT2->actionText = actionTextT2;
    tool2 = new ToolDocument( "tool2", controller, factoryT2 );
    viewT21 = tool2->createView();
    area->addToolView( viewT21, Sublime::Left );

    controller->showArea(area, mw);
}

void TestToolViewToolBar::cleanup()
{
    delete controller;
}

QToolBar* TestToolViewToolBar::fetchToolBarFor(Sublime::View* view)
{
    QWidget* toolWidget = view->widget();
    const char* loc = "fetchToolBarFor";
    Q_UNUSED(loc);
    Q_ASSERT_X(toolWidget, loc, "Tool refuses to create widget (null).");
    Q_ASSERT(toolWidget->parent());
    QMainWindow* toolWin = dynamic_cast<QMainWindow*>(toolWidget->parent());
    Q_ASSERT_X(toolWin, loc, "Tool widget's parent is not a QMainWindow.");
    QList<QToolBar*> toolBars = toolWin->findChildren<QToolBar*>();
    int barCount = toolBars.count();
    char* failMsg = qstrdup(QString("Expected to find a toolbar but found %1").arg(barCount).toLatin1().data());
    Q_UNUSED(failMsg);
    Q_ASSERT_X(barCount == 1, loc, failMsg);
    return toolBars.at(0);
}

void TestToolViewToolBar::assertGoodBar(QToolBar* toolbar, QString actionText)
{
    QVERIFY( toolbar );
    QVERIFY( !toolbar->isFloatable() );
    QCOMPARE( toolbar->iconSize(), QSize( 16, 16 ) );
    QList<QAction*> actions = toolbar->actions();
    QCOMPARE( actions.count(), 1 );
    QCOMPARE( actions.at(0)->text(), actionText);
    QCOMPARE( toolbar->orientation(), Qt::Horizontal );
}

void TestToolViewToolBar::horizontalTool()
{
    // viewT11 was added with Sublime::Bottom, so it should have a horizontal bar
    QToolBar* bar = fetchToolBarFor(viewT11);
    assertGoodBar(bar, actionTextT1);
}

void TestToolViewToolBar::verticalTool()
{
    // viewT21 was added with Sublime::Left, so it should have a vertical bar
    QToolBar* bar = fetchToolBarFor(viewT21);
    assertGoodBar(bar, actionTextT2);
}

void TestToolViewToolBar::toolViewMove()
{
    area->moveToolView( viewT11, Sublime::Right );
    area->moveToolView( viewT21, Sublime::Bottom );
    QToolBar* barT1 = fetchToolBarFor(viewT11);
    QToolBar* barT2 = fetchToolBarFor(viewT21);
    assertGoodBar(barT1, actionTextT1);
    assertGoodBar(barT2, actionTextT2);
}

QTEST_MAIN(TestToolViewToolBar)

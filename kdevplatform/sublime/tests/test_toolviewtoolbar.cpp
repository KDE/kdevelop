/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2008 Manuel Breugelmans <mbr.nxi@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_toolviewtoolbar.h"

#include <QTest>

#include <QTextEdit>
#include <QToolBar>

#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>
#include <sublime/mainwindow.h>
#include <sublime/tooldocument.h>
#include <tests/corelesshelpers.h>

using namespace Sublime;

class ToolViewToolBarFactory : public SimpleToolWidgetFactory<QTextEdit> {
public:
    explicit ToolViewToolBarFactory(const QString &id): SimpleToolWidgetFactory<QTextEdit>(id) {}
    QList<QAction*> toolBarActions( QWidget* ) const override
    {
        return actionList;
    }

    ~ToolViewToolBarFactory() override
    {
        qDeleteAll(actionList);
    }

    void addAction(const QString &text)
    {
        auto* action = new QAction(text, nullptr);
        actionList.append(action);
    }

private:
    QList<QAction*> actionList;
};

void TestToolViewToolBar::initTestCase()
{
    KDevelop::initCorelessTestCase();
}

void TestToolViewToolBar::init()
{
    // this is starting to become a GeneralFixture
    controller = new Controller(this);
    area = new Area( controller, QStringLiteral("Area") );
    auto* mw = new MainWindow(controller);

    // a horizontal tool with toolbar
    auto* factoryT1 = new ToolViewToolBarFactory(QStringLiteral("tool1factory"));
    actionTextT1 = QStringLiteral("Tool1Action");
    factoryT1->addAction(actionTextT1);
    tool1 = new ToolDocument( QStringLiteral("tool1"), controller, factoryT1 );
    viewT11 = tool1->createView();
    area->addToolView( viewT11, Sublime::Bottom );

    // a vertical tool with toolbar
    auto* factoryT2 = new ToolViewToolBarFactory(QStringLiteral("tool2factory"));
    actionTextT2 = QStringLiteral("Tool2Action");
    factoryT2->addAction(actionTextT2);
    tool2 = new ToolDocument( QStringLiteral("tool2"), controller, factoryT2 );
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
    auto* toolWin = qobject_cast<QMainWindow*>(toolWidget->parent());
    Q_ASSERT_X(toolWin, loc, "Tool widget's parent is not a QMainWindow.");
    QList<QToolBar*> toolBars = toolWin->findChildren<QToolBar*>();
    int barCount = toolBars.count();
    char* failMsg = qstrdup(QStringLiteral("Expected to find a toolbar but found %1").arg(barCount).toLatin1().data());
    Q_UNUSED(failMsg);
    Q_ASSERT_X(barCount == 1, loc, failMsg);
    delete [] failMsg;
    return toolBars.at(0);
}

void TestToolViewToolBar::assertGoodBar(QToolBar* toolbar, const QString& actionText)
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
    area->setToolViewPosition(viewT11, Sublime::Right);
    area->setToolViewPosition(viewT21, Sublime::Bottom);
    QToolBar* barT1 = fetchToolBarFor(viewT11);
    QToolBar* barT2 = fetchToolBarFor(viewT21);
    assertGoodBar(barT1, actionTextT1);
    assertGoodBar(barT2, actionTextT2);
}

QTEST_MAIN(TestToolViewToolBar)

#include "moc_test_toolviewtoolbar.cpp"

/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_viewactivation.h"

#include <QTest>
#include <QSignalSpy>

#include <QListView>
#include <QTextEdit>
#include <QDockWidget>
#include <QFocusEvent>

#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>
#include <sublime/mainwindow.h>
#include <sublime/container.h>
#include <sublime/tooldocument.h>
#include <tests/corelesshelpers.h>

using namespace Sublime;

template <class Widget>
class SpecialWidgetFactory: public SimpleToolWidgetFactory<Widget> {
public:
    explicit SpecialWidgetFactory(const QString &id): SimpleToolWidgetFactory<Widget>(id) {}
    QWidget* create(ToolDocument *doc, QWidget *parent = nullptr) override
    {
        auto* w = new QWidget(parent);
        auto *inner = new Widget(w);
        inner->setObjectName(doc->title()+"_inner");
        w->setObjectName(doc->title()+"_outer");
        return w;
    }
};

void TestViewActivation::initTestCase()
{
    KDevelop::initCorelessTestCase();
    qRegisterMetaType<View*>("View*");
}

void TestViewActivation::init()
{
    controller = new Controller(this);
    doc1 = new ToolDocument(QStringLiteral("doc1"), controller, new SimpleToolWidgetFactory<QListView>(QStringLiteral("doc1")));
    //this document will create special widgets - QListView nested in QWidget
    doc2 = new ToolDocument(QStringLiteral("doc2"), controller, new SpecialWidgetFactory<QListView>(QStringLiteral("doc2")));
    doc3 = new ToolDocument(QStringLiteral("doc3"), controller, new SimpleToolWidgetFactory<QListView>(QStringLiteral("doc3")));
    doc4 = new ToolDocument(QStringLiteral("doc4"), controller, new SimpleToolWidgetFactory<QListView>(QStringLiteral("doc4")));

    tool1 = new ToolDocument(QStringLiteral("tool1"), controller, new SimpleToolWidgetFactory<QListView>(QStringLiteral("tool1")));
    tool2 = new ToolDocument(QStringLiteral("tool2"), controller, new SimpleToolWidgetFactory<QTextEdit>(QStringLiteral("tool2")));
    tool3 = new ToolDocument(QStringLiteral("tool3"), controller, new SimpleToolWidgetFactory<QTextEdit>(QStringLiteral("tool3")));

    area = new Area(controller, QStringLiteral("Area"));

    view211 = doc1->createView();
    view211->setObjectName(QStringLiteral("view211"));
    area->addView(view211);
    view212 = doc1->createView();
    view212->setObjectName(QStringLiteral("view212"));
    area->addView(view212);
    view221 = doc2->createView();
    area->addView(view221, view211, Qt::Vertical);
    view231 = doc3->createView();
    area->addView(view231, view221, Qt::Horizontal);
    view241 = doc4->createView();
    area->addView(view241, view212, Qt::Vertical);
    viewT11 = tool1->createView();
    area->addToolView(viewT11, Sublime::Bottom);
    viewT21 = tool2->createView();
    area->addToolView(viewT21, Sublime::Right);
    viewT31 = tool3->createView();
    area->addToolView(viewT31, Sublime::Top);
    viewT32 = tool3->createView();
    area->addToolView(viewT32, Sublime::Top);
}

void TestViewActivation::cleanup()
{
    delete controller;
}

void TestViewActivation::signalsOnViewCreationAndDeletion()
{
    auto *controller = new Controller(this);
    auto* doc1 = new ToolDocument(QStringLiteral("doc1"), controller, new SimpleToolWidgetFactory<QListView>(QStringLiteral("doc1")));
    Area *area = new Area(controller, QStringLiteral("Area"));

    QSignalSpy spy(controller, SIGNAL(viewAdded(Sublime::View*)));
    View *v = doc1->createView();
    area->addView(v);
    QCOMPARE(spy.count(), 1);

    QSignalSpy spy2(controller, SIGNAL(aboutToRemoveView(Sublime::View*)));
    area->removeView(v);
    QCOMPARE(spy2.count(), 1);

    QSignalSpy spy3(controller, SIGNAL(toolViewAdded(Sublime::View*)));
    v = doc1->createView();
    area->addToolView(v, Sublime::Bottom);
    QCOMPARE(spy3.count(), 1);

    QSignalSpy spy4(controller, SIGNAL(aboutToRemoveToolView(Sublime::View*)));
    area->removeToolView(v);
    QCOMPARE(spy4.count(), 1);

    delete controller;
}

void TestViewActivation::viewActivation()
{
    auto* mw = new MainWindow(controller);
    controller->addDefaultArea(area); // Q_ASSERT without this.
    controller->addMainWindow(mw);

    controller->showArea(area, mw);
    //we should have an active view immediately after the area is shown
    QCOMPARE(mw->activeView(), view211);

    //add some widgets that are not in layout
    auto *breaker = new QTextEdit(mw);
    breaker->setObjectName(QStringLiteral("breaker"));
    auto *toolBreaker = new QTextEdit(mw);
    toolBreaker->setObjectName(QStringLiteral("toolBreaker"));

    auto* dock = new QDockWidget(mw);
    dock->setWidget(toolBreaker);
    mw->addDockWidget(Qt::LeftDockWidgetArea, dock);

    QFocusEvent focusEvent(QEvent::FocusIn);
    //now post events to the widgets and see if mainwindow has the right active views
    //activate view
    QVERIFY(view212->widget());
    qApp->sendEvent(view212->widget(), &focusEvent);
    QString failMsg = QStringLiteral("\nWas expecting %1 to be active but got %2").
                      arg(view212->objectName(), mw->activeView()->objectName());
    QVERIFY2(mw->activeView() == view212, failMsg.toLatin1().data());

    //activate tool view and check that both view and tool view are active
    QVERIFY(viewT31->widget());
    qApp->sendEvent(viewT31->widget(), &focusEvent);
    QCOMPARE(mw->activeView(), view212);
    QCOMPARE(mw->activeToolView(), viewT31);

    //active another view
    QVERIFY(view241->widget());
    qApp->sendEvent(view241->widget(), &focusEvent);
    QCOMPARE(mw->activeView(), view241);
    QCOMPARE(mw->activeToolView(), viewT31);

    //focus a widget not in the area
    qApp->sendEvent(breaker, &focusEvent);
    QCOMPARE(mw->activeView(), view241);
    QCOMPARE(mw->activeToolView(), viewT31);

    //focus a dock not in the area
    qApp->sendEvent(toolBreaker, &focusEvent);
    QCOMPARE(mw->activeView(), view241);
    QCOMPARE(mw->activeToolView(), viewT31);

    //focus inner widget for view221
    auto *inner = mw->findChild<QListView*>(QStringLiteral("doc2_inner"));
    QVERIFY(inner);
    qApp->sendEvent(inner, &focusEvent);
    QCOMPARE(mw->activeView(), view221);
    QCOMPARE(mw->activeToolView(), viewT31);
}

void TestViewActivation::activationInMultipleMainWindows()
{
    MainWindow mw(controller);
    controller->showArea(area, &mw);
    QCOMPARE(mw.activeView(), view211);

    //check that new mainwindow always have active view right after displaying area
    MainWindow mw2(controller);
    controller->showArea(area, &mw2);
    QVERIFY(mw2.activeView());
    QCOMPARE(mw2.activeView()->document(), doc1);
}

void TestViewActivation::activationAfterViewRemoval()
{
    MainWindow mw(controller);
    controller->showArea(area, &mw);
    QCOMPARE(mw.activeView(), view211);

    //check what happens if we remove a view which is not the only one in container
    delete area->removeView(view211);
    QCOMPARE(mw.activeView(), view212);

    //check what happens if we remove a view which is alone in container
    mw.activateView(view231);
    QCOMPARE(mw.activeView(), view231);
    delete area->removeView(view231);
    QCOMPARE(mw.activeView(), view221);
}

void TestViewActivation::activationAfterRemovalSimplestCase()
{
    //we don't have split views - just two views in one area index
    MainWindow mw(controller);
    Area *area = new Area(controller, QStringLiteral("Area"));
    View *v1 = doc1->createView();
    View *v2 = doc2->createView();
    area->addView(v1);
    area->addView(v2, v1);
    controller->showArea(area, &mw);
    mw.activateView(v2);

    //delete active view and check that previous is activated
    delete area->removeView(v2);
    QCOMPARE(mw.activeView(), v1);
}

QTEST_MAIN(TestViewActivation)

#include "moc_test_viewactivation.cpp"

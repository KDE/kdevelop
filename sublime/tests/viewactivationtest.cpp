/***************************************************************************
 *   Copyright 2007 Alexander Dymo  <adymo@kdevelop.org>            *
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
#include "viewactivationtest.h"

#include <QtTest/QtTest>

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

#include "kdebug.h"

using namespace Sublime;

template <class Widget>
class SpecialWidgetFactory: public SimpleToolWidgetFactory<Widget> {
public:
    SpecialWidgetFactory(const QString &id): SimpleToolWidgetFactory<Widget>(id) {}
    virtual QWidget* create(ToolDocument *doc, QWidget *parent = 0)
    {
        QWidget *w = new QWidget(parent);
        Widget *inner = new Widget(w);
        inner->setObjectName(doc->title()+"_inner");
        w->setObjectName(doc->title()+"_outer");
        return w;
    }
};

void ViewActivationTest::initTestCase()
{
    qRegisterMetaType<View*>("View*");
}

void ViewActivationTest::init()
{
    controller = new Controller(this);
    doc1 = new ToolDocument("doc1", controller, new SimpleToolWidgetFactory<QListView>("doc1"));
    //this document will create special widgets - QListView nested in QWidget
    doc2 = new ToolDocument("doc2", controller, new SpecialWidgetFactory<QListView>("doc2"));
    doc3 = new ToolDocument("doc3", controller, new SimpleToolWidgetFactory<QListView>("doc3"));
    doc4 = new ToolDocument("doc4", controller, new SimpleToolWidgetFactory<QListView>("doc4"));

    tool1 = new ToolDocument("tool1", controller, new SimpleToolWidgetFactory<QListView>("tool1"));
    tool2 = new ToolDocument("tool2", controller, new SimpleToolWidgetFactory<QTextEdit>("tool2"));
    tool3 = new ToolDocument("tool3", controller, new SimpleToolWidgetFactory<QTextEdit>("tool3"));

    area = new Area(controller, "Area");

    view211 = doc1->createView();
    view211->setObjectName("view211");
    area->addView(view211);
    view212 = doc1->createView();
    view212->setObjectName("view212");
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

void ViewActivationTest::cleanup()
{
    delete controller;
}

void ViewActivationTest::signalsOnViewCreationAndDeletion()
{
    Controller *controller = new Controller(this);
    ToolDocument *doc1 = new ToolDocument("doc1", controller, new SimpleToolWidgetFactory<QListView>("doc1"));
    Area *area = new Area(controller, "Area");

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

void ViewActivationTest::viewActivation()
{
    MainWindow* mw = new MainWindow(controller);
    controller->addDefaultArea(area); // Q_ASSERT without this.
    controller->addMainWindow(mw);

    controller->showArea(area, mw);
    //we should have an active view immediatelly after the area is shown
    QCOMPARE(mw->activeView(), view211);

    //add some widgets that are not in layout
    QTextEdit *breaker = new QTextEdit(mw);
    breaker->setObjectName("breaker");
    QTextEdit *toolBreaker = new QTextEdit(mw);
    toolBreaker->setObjectName("toolBreaker");

    QDockWidget *dock = new QDockWidget(mw);
    dock->setWidget(toolBreaker);
    mw->addDockWidget(Qt::LeftDockWidgetArea, dock);

    //now post events to the widgets and see if mainwindow has the right active views
    //activate view
    qApp->sendEvent(view212->widget(), new QFocusEvent(QEvent::FocusIn));
    QString failMsg = QString("\nWas expecting %1 to be active but got %2").
                      arg(view212->objectName()).arg(mw->activeView()->objectName());
    QVERIFY2(mw->activeView() == view212, failMsg.toAscii().data());

    //activate toolview and check that both view and toolview are active
    qApp->sendEvent(viewT31->widget(), new QFocusEvent(QEvent::FocusIn));
    QCOMPARE(mw->activeView(), view212);
    QCOMPARE(mw->activeToolView(), viewT31);

    //active another view
    qApp->sendEvent(view241->widget(), new QFocusEvent(QEvent::FocusIn));
    QCOMPARE(mw->activeView(), view241);
    QCOMPARE(mw->activeToolView(), viewT31);

    //focus a widget not in the area
    qApp->sendEvent(breaker, new QFocusEvent(QEvent::FocusIn));
    QCOMPARE(mw->activeView(), view241);
    QCOMPARE(mw->activeToolView(), viewT31);

    //focus a dock not in the area
    qApp->sendEvent(toolBreaker, new QFocusEvent(QEvent::FocusIn));
    QCOMPARE(mw->activeView(), view241);
    QCOMPARE(mw->activeToolView(), viewT31);

    //focus inner widget for view221
    QListView *inner = mw->findChild<QListView*>("doc2_inner");
    QVERIFY(inner);
    qApp->sendEvent(inner, new QFocusEvent(QEvent::FocusIn));
    QCOMPARE(mw->activeView(), view221);
    QCOMPARE(mw->activeToolView(), viewT31);
}

void ViewActivationTest::activationInMultipleMainWindows()
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

void ViewActivationTest::activationAfterViewRemoval()
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

void ViewActivationTest::activationAfterRemovalSimplestCase()
{
    //we don't have split views - just two views in one area index
    MainWindow mw(controller);
    Area *area = new Area(controller, "Area");
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

QTEST_MAIN(ViewActivationTest)
#include "viewactivationtest.moc"

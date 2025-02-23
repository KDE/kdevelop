/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test_controller.h"

#include <QTextEdit>
#include <QTest>
#include <QDebug>
#include <QStandardPaths>

#include <sublime/controller.h>
#include <sublime/tooldocument.h>
#include <sublime/view.h>
#include <sublime/area.h>

using namespace Sublime;

void TestController::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestController::documentDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument(QStringLiteral("tool"), &controller, new SimpleToolWidgetFactory<QTextEdit>(QStringLiteral("tool")));
    delete doc;
}

void TestController::areaDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument(QStringLiteral("tool"), &controller, new SimpleToolWidgetFactory<QTextEdit>(QStringLiteral("tool")));
    //create a view which does not belong to an area
    View* view1 = doc->createView();
    Q_UNUSED(view1);
    //create an area and two views in it
    Area *area = new Area(&controller, QStringLiteral("MyArea"));
    controller.addDefaultArea(area);
    QCOMPARE(controller.defaultAreas().count(), 1);
    View* view2 = doc->createView();
    view2->setObjectName(QStringLiteral("VIEW2"));
    area->addView(view2);
    View* view3 = doc->createView();
    view3->setObjectName(QStringLiteral("VIEW3"));
    area->addView(view3);
    QCOMPARE(doc->views().count(), 3);
    QCOMPARE(area->views().count(), 2);

    delete area;
    view2 = nullptr; view3= nullptr;

    QEXPECT_FAIL("", "Fails because of delayed view deletion", Continue);
    QCOMPARE(doc->views().count(), 1);
    QCOMPARE(controller.defaultAreas().count(), 0);

    QTest::qWait(100); // wait for deleteLaters
    qDebug() << "Deleting doc";
    delete doc;
    QTest::qWait(100); // wait for deleteLaters
    qDebug() << "View2 & view3 are destructored at this point (but no earlier).";
}

void TestController::namedAreas()
{
    Controller controller;
    Area *area1 = new Area(&controller, QStringLiteral("1"));
    controller.addDefaultArea(area1);
    Area *area2 = new Area(&controller, QStringLiteral("2"));
    controller.addDefaultArea(area2);

    QCOMPARE(controller.defaultArea(QStringLiteral("1")), area1);
    QCOMPARE(controller.defaultArea(QStringLiteral("2")), area2);
}

QTEST_MAIN(TestController)

#include "moc_test_controller.cpp"

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
#include "test_controller.h"

#include <QTextEdit>
#include <QTest>
#include <QDebug>

#include <sublime/controller.h>
#include <sublime/tooldocument.h>
#include <sublime/view.h>
#include <sublime/area.h>

using namespace Sublime;

void TestController::documentDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument(QStringLiteral("tool"), &controller, new SimpleToolWidgetFactory<QTextEdit>(QStringLiteral("tool")));
    QCOMPARE(controller.documents().count(), 1);
    delete doc;
    QCOMPARE(controller.documents().count(), 0);
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

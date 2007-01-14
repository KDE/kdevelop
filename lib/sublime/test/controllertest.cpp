/***************************************************************************
 *   Copyright (C) 2007 by Alexander Dymo  <adymo@kdevelop.org>            *
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
#include "controllertest.h"

#include <QTextEdit>
#include <QtTest/QtTest>

#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>

#include <sublime/controller.h>
#include <sublime/tooldocument.h>
#include <sublime/view.h>
#include <sublime/area.h>

#include "kdevtest.h"

using namespace Sublime;

void ControllerTest::testDocumentDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument(&controller, new SimpleToolWidgetFactory<QTextEdit>());
    QCOMPARE(controller.documents().count(), 1);
    delete doc;
    QCOMPARE(controller.documents().count(), 0);
}

void ControllerTest::testAreaDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument(&controller, new SimpleToolWidgetFactory<QTextEdit>());
    //create a view which does not belong to an area
    doc->createView();
    //create an area and two views in it
    Area *area = new Area(&controller, "MyArea");
    QCOMPARE(controller.areas().count(), 1);
    area->addView(doc->createView());
    area->addView(doc->createView());
    QCOMPARE(doc->views().count(), 3);
    QCOMPARE(area->views().count(), 2);

    delete area;
    //now we should have only one view remaining view that did not belong to the area
    //but we still have 3 because deleteLater only queues the deletion
    QCOMPARE(doc->views().count(), 3);
    QCOMPARE(controller.areas().count(), 0);
}

#include "controllertest.moc"
KDEVTEST_MAIN(ControllerTest)

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

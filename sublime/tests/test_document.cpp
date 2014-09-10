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
#include "test_document.h"

#include <QTextEdit>
#include <QtTest/QtTest>

#include <sublime/controller.h>
#include <sublime/tooldocument.h>
#include <sublime/view.h>

using namespace Sublime;

void TestDocument::viewDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument("tool", &controller, new SimpleToolWidgetFactory<QTextEdit>("tool"));

    View *view = doc->createView();
    view->widget();
    QCOMPARE(doc->views().count(), 1);

    delete view;
    QCOMPARE(doc->views().count(), 0);
}

QTEST_MAIN(TestDocument)

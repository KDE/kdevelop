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
#include "viewtest.h"

#include <QTextEdit>
#include <QtTest/QtTest>

#include <kurl.h>

#include <sublime/controller.h>
#include <sublime/tooldocument.h>
#include <sublime/view.h>

#include "kdevtest.h"

using namespace Sublime;

void ViewTest::testWidgetDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument(&controller, new SimpleToolWidgetFactory<QTextEdit>());

    View *view = doc->createView();
    //create the widget
    view->widget();
    QVERIFY(view->hasWidget());
    QCOMPARE(view->widget()->metaObject()->className(), "QTextEdit");

    //delete the widget and check that view knows about that
    delete view->widget();
    QVERIFY(!view->hasWidget());
}

#include "viewtest.moc"
KDEVTEST_MAIN(ViewTest)

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

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
#include "test_view.h"

#include <QTextEdit>
#include <QtTest/QtTest>

#include <sublime/controller.h>
#include <sublime/tooldocument.h>
#include <sublime/view.h>

using namespace Sublime;

void TestView::widgetDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument("tool", &controller, new SimpleToolWidgetFactory<QTextEdit>("tool"));

    View *view = doc->createView();
    //create the widget
    view->widget();
    QVERIFY(view->hasWidget());
    QCOMPARE(view->widget()->metaObject()->className(), "QTextEdit");

    //delete the widget and check that view knows about that
    delete view->widget();
    QVERIFY(!view->hasWidget());
}

class Test: public View {
public:
    Test(Document *doc): View(doc) {}
};

class TestDocument: public Document {
public:
    TestDocument(Controller *controller): Document("TestDocument", controller) {}
    virtual QString documentType() const { return "Test"; }
    virtual QString documentSpecifier() const { return QString(); }
protected:
    virtual QWidget *createViewWidget(QWidget *parent = 0) { return new QWidget(parent); }
    virtual View *newView(Document *doc) { return new Test(doc); }
};

void TestView::viewReimplementation()
{
    Controller controller;
    Document *doc = new TestDocument(&controller);
    View *view = doc->createView();
    QVERIFY(dynamic_cast<Test*>(view) != 0);
}

QTEST_MAIN(TestView)

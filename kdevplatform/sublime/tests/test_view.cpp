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
#include <QTest>
#include <QStandardPaths>

#include <sublime/controller.h>
#include <sublime/tooldocument.h>
#include <sublime/view.h>

using namespace Sublime;

void TestView::widgetDeletion()
{
    Controller controller;
    Document *doc = new ToolDocument(QStringLiteral("tool"), &controller, new SimpleToolWidgetFactory<QTextEdit>(QStringLiteral("tool")));

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
Q_OBJECT
public:
    explicit Test(Document *doc): View(doc) {}
};

class TestDocument: public Document {
Q_OBJECT
public:
    explicit TestDocument(Controller *controller): Document(QStringLiteral("TestDocument"), controller) {}
    QString documentType() const override { return QStringLiteral("Test"); }
    QString documentSpecifier() const override { return QString(); }
protected:
    QWidget *createViewWidget(QWidget *parent = nullptr) override { return new QWidget(parent); }
    View *newView(Document *doc) override { return new Test(doc); }
};

void TestView::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestView::viewReimplementation()
{
    Controller controller;
    Document *doc = new TestDocument(&controller);
    View *view = doc->createView();
    QVERIFY(dynamic_cast<Test*>(view) != nullptr);
}

QTEST_MAIN(TestView)

#include "test_view.moc"
